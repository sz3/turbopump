/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MessageSender.h"

#include "cohesion/TreeId.h"
#include "common/MerklePoint.h"
#include "membership/Peer.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockPeerTracker.h"

#include "socket/IpAddress.h"

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
	assertEquals( "write(0,sync|tree=oak n=3|1 2 3)|flush()", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testDigestPing.Null", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.digestPing(Peer("dude"), TreeId("oak"), MerklePoint::null());

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,sync|tree=oak n=3|0 65535 0)|flush()", writer->_history.calls() );
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
	assertEquals( "write(0,sync|tree=oak n=2|1 1 10|2 2 20|3 3 30)|flush()", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testRequestKeyRange", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.requestKeyRange(Peer("foo"), TreeId("oak",2), 1234, 5678);

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "write(0,key-req|tree=oak n=2 first=1234 last=5678|)|flush()", writer->_history.calls() );
}
