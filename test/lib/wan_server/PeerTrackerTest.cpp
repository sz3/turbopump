/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "PeerTracker.h"

#include "IBufferedConnectionWriter.h"
#include "membership/Peer.h"
#include "mock/MockPacketServer.h"
#include "mock/MockIpSocket.h"
using std::shared_ptr;
using std::string;

TEST_CASE( "PeerTrackerTest/testGetWriter", "[unit]" )
{
	MockPacketServer server;
	server._sock.reset(new MockIpSocket);
	PeerTracker tracker(server);

	Peer peer("fooid");
	peer.ips.push_back("1.2.3.4:1234");

	shared_ptr<IBufferedConnectionWriter> writer = tracker.getWriter(peer);
	assertTrue( writer );

	assertEquals( "sock(1.2.3.4:1234)", server._history.calls() );
}

TEST_CASE( "PeerTrackerTest/testGetWriter.BadPeerAddress", "[unit]" )
{
	MockPacketServer server;
	server._sock.reset(new MockIpSocket);
	PeerTracker tracker(server);

	Peer peer("fooid");

	shared_ptr<IBufferedConnectionWriter> writer = tracker.getWriter(peer);
	assertFalse( writer );

	assertEquals( "", server._history.calls() );
}

TEST_CASE( "PeerTrackerTest/testGetWriter.BadConnection", "[unit]" )
{
	MockPacketServer server;
	PeerTracker tracker(server);

	Peer peer("fooid");
	peer.ips.push_back("1.2.3.4:1234");

	shared_ptr<IBufferedConnectionWriter> writer = tracker.getWriter(peer);
	assertFalse( writer );

	assertEquals( "sock(1.2.3.4:1234)", server._history.calls() );
}

