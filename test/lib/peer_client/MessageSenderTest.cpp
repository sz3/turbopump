/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MessageSender.h"

#include "common/MerklePoint.h"
#include "deskew/TreeId.h"
#include "membership/Peer.h"
#include "mock/MockRequestPacker.h"
#include "socket/MockSocketServer.h"
#include "socket/MockSocketWriter.h"

namespace {
	Peer mockPeer(std::string ip)
	{
		Peer peer("foo");
		peer.ips.push_back(ip);
		return peer;
	}
}

TEST_CASE( "MessageSenderTest/testDigestPing", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	MessageSender messenger(packer, server);

	MerklePoint point;
	point.key = 1;
	point.keybits = 2;
	point.hash = 3;
	messenger.digestPing(mockPeer("1.2.3.4:80"), TreeId("oak"), point);

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(102)", packer._history.calls() );
	assertEquals( "try_send({102 id=oak mirrors=3}1 2 3)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDigestPing.Null", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	MessageSender messenger(packer, server);
	messenger.digestPing(mockPeer("1.2.3.4:80"), TreeId("oak"), MerklePoint::null());

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(102)", packer._history.calls() );
	assertEquals( "try_send({102 id=oak mirrors=3}0 65535 0)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDigestPing.Many", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	MessageSender messenger(packer, server);

	std::deque<MerklePoint> points;
	for (int i = 1; i <= 3; ++i)
	{
		MerklePoint point;
		point.key = i;
		point.keybits = i;
		point.hash = i*10;
		points.push_back(point);
	}
	messenger.digestPing(mockPeer("1.2.3.4:80"), TreeId("oak",2), points);

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(102)", packer._history.calls() );
	assertEquals( "try_send({102 id=oak mirrors=2}1 1 10|2 2 20|3 3 30)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testRequestKeyRange", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	MessageSender messenger(packer, server);
	messenger.requestKeyRange(mockPeer("1.2.3.4:80"), TreeId("oak",2), 1234, 5678);

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(103)", packer._history.calls() );
	assertEquals( "try_send({103 first=1234 id=oak last=5678 mirrors=2})|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testOfferWrite", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	MessageSender messenger(packer, server);
	messenger.offerWrite(mockPeer("1.2.3.4:80"), "file1", "version1", "source1");

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(105)", packer._history.calls() );
	assertEquals( "try_send({105 name=file1 source=source1 version=version1})|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDemandWrite", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	MessageSender messenger(packer, server);
	messenger.demandWrite(mockPeer("1.2.3.4:80"), "file1", "version1", "source1");

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(106)", packer._history.calls() );
	assertEquals( "try_send({106 name=file1 source=source1 version=version1})|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testAcknowledgeWrite", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	MessageSender messenger(packer, server);
	messenger.acknowledgeWrite(mockPeer("1.2.3.4:80"), "file1", "version1", 1234);

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(101)", packer._history.calls() );
	assertEquals( "send({101 name=file1 size=1234 version=version1})|flush(true)", writer->_history.calls() );
}
