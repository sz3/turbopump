/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MessageSender.h"

#include "common/MerklePoint.h"
#include "deskew/TreeId.h"
#include "membership/Peer.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockPeerTracker.h"
#include "mock/MockRequestPacker.h"

TEST_CASE( "MessageSenderTest/testDigestPing", "[unit]" )
{
	MockRequestPacker packer;
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(packer, peers);

	MerklePoint point;
	point.location.key = 1;
	point.location.keybits = 2;
	point.hash = 3;
	messenger.digestPing(Peer("dude"), TreeId("oak"), point);

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "package(102)", packer._history.calls() );
	assertEquals( "write(0,{102 id=oak mirrors=3}1 2 3,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDigestPing.Null", "[unit]" )
{
	MockRequestPacker packer;
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(packer, peers);
	messenger.digestPing(Peer("dude"), TreeId("oak"), MerklePoint::null());

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "package(102)", packer._history.calls() );
	assertEquals( "write(0,{102 id=oak mirrors=3}0 65535 0,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDigestPing.Many", "[unit]" )
{
	MockRequestPacker packer;
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(packer, peers);

	std::deque<MerklePoint> points;
	for (int i = 1; i <= 3; ++i)
	{
		MerklePoint point;
		point.location.key = i;
		point.location.keybits = i;
		point.hash = i*10;
		points.push_back(point);
	}
	messenger.digestPing(Peer("dude"), TreeId("oak",2), points);

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "package(102)", packer._history.calls() );
	assertEquals( "write(0,{102 id=oak mirrors=2}1 1 10|2 2 20|3 3 30,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testRequestKeyRange", "[unit]" )
{
	MockRequestPacker packer;
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(packer, peers);
	messenger.requestKeyRange(Peer("foo"), TreeId("oak",2), 1234, 5678);

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "package(103)", packer._history.calls() );
	assertEquals( "write(0,{103 first=1234 id=oak last=5678 mirrors=2},false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testOfferWrite", "[unit]" )
{
	MockRequestPacker packer;
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(packer, peers);
	messenger.offerWrite(Peer("foo"), "file1", "version1", "source1");

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "package(105)", packer._history.calls() );
	assertEquals( "write(0,{105 name=file1 source=source1 version=version1},false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDemandWrite", "[unit]" )
{
	MockRequestPacker packer;
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(packer, peers);
	messenger.demandWrite(Peer("foo"), "file1", "version1", "source1");

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "package(106)", packer._history.calls() );
	assertEquals( "write(0,{106 name=file1 source=source1 version=version1},false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testAcknowledgeWrite", "[unit]" )
{
	MockRequestPacker packer;
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(packer, peers);
	messenger.acknowledgeWrite(Peer("foo"), "file1", "version1", 1234);

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "package(101)", packer._history.calls() );
	assertEquals( "write(0,{101 name=file1 size=1234 version=version1},true)|flush(true)", writer->_history.calls() );
}