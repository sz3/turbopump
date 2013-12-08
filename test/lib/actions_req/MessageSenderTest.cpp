#include "unittest.h"

#include "MessageSender.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockPeerTracker.h"

#include "socket/IpAddress.h"

TEST_CASE( "MessageSenderTest/testMerklePing", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);

	MerklePoint point;
	point.location.key = 1;
	point.location.keybits = 2;
	point.hash = 3;
	messenger.merklePing(Peer("dude"), point);

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,merkle||1 2 3)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testMerklePing.Null", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.merklePing(Peer("dude"), MerklePoint::null());

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,merkle||0 65535 0)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testMerklePing.Many", "[unit]" )
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
	messenger.merklePing(Peer("dude"), points);

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,merkle||1 1 10|2 2 20|3 3 30)", writer->_history.calls() );
}

TEST_CASE( "MessageSenderTest/testRequestKeyRange", "[unit]" )
{
	MockPeerTracker peers;
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	MessageSender messenger(peers);
	messenger.requestKeyRange(Peer("foo"), 1234, 5678);

	assertEquals( "getWriter(foo)", peers._history.calls() );
	assertEquals( "write(0,key-req|first=1234 last=5678|)", writer->_history.calls() );
}
