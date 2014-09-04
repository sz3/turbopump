/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MessageSender.h"

#include "common/MerklePoint.h"
#include "deskew/TreeId.h"
#include "membership/Peer.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockPeerTracker.h"

TEST_CASE( "MessageSenderTest/testDigestPing", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);

	MerklePoint point;
	point.location.key = 1;
	point.location.keybits = 2;
	point.hash = 3;
	messenger.digestPing(Peer("dude"), TreeId("oak"), point);

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,sync|tree=oak n=3|1 2 3,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDigestPing.Null", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.digestPing(Peer("dude"), TreeId("oak"), MerklePoint::null());

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,sync|tree=oak n=3|0 65535 0,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDigestPing.Many", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);

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
	assertEquals( "write(0,sync|tree=oak n=2|1 1 10|2 2 20|3 3 30,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testRequestKeyRange", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.requestKeyRange(Peer("foo"), TreeId("oak",2), 1234, 5678);

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "write(0,key-req|tree=oak n=2 first=1234 last=5678|,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testOfferWrite", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.offerWrite(Peer("foo"), "file1", "version1", "source1");

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "write(0,offer-write|name=file1 v=version1 source=source1|,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDemandWrite", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.demandWrite(Peer("foo"), "file1", "version1", "source1");

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "write(0,demand-write|name=file1 v=version1 source=source1|,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testAcknowledgeWrite", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.acknowledgeWrite(Peer("foo"), "file1", "version1", 1234);

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "write(0,ack-write|name=file1 v=version1 size=1234|,true)|flush(true)", writer->_history.calls() );
}
