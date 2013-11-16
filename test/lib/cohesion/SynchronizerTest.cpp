#include "unittest.h"

#include "Synchronizer.h"

#include "MerklePoint.h"
#include "membership/Peer.h"
#include "mock/MockMembership.h"
#include "mock/MockMerkleIndex.h"
#include "mock/MockPeerTracker.h"
#include "mock/MockIpSocket.h"
#include "mock/MockSkewCorrector.h"
#include "socket/IpAddress.h"
#include "wan_server/PeerConnection.h"

TEST_CASE( "SynchronizerTest/testPingRandomPeer", "default" )
{
	MockMembership membership;
	membership.addIp("1.2.3.4", "dude");
	MockPeerTracker peers;
	MockMerkleIndex index;
	MockSkewCorrector corrector;

	MockIpSocket* mockSock = new MockIpSocket();
	std::shared_ptr<IIpSocket> sock(mockSock);
	peers._conn.reset(new PeerConnection(sock));

	Synchronizer sinkro(membership, peers, index, corrector);
	sinkro.pingRandomPeer();

	assertEquals( "addIp(1.2.3.4,dude)|randomPeer()", membership._history.calls() );
	assertEquals( "track(dude)", peers._history.calls() );
	assertEquals( "send(ping||)", mockSock->_history.calls() );
}

namespace {
	MerklePoint whatsThePoint(unsigned value)
	{
		MerklePoint point;
		point.location.key = value;
		point.location.keybits = value;
		point.hash = value * 10;
		return point;
	}
}

TEST_CASE( "SynchronizerTest/testCompare.Empty", "default" )
{
	MockMembership membership;
	MockPeerTracker peers;
	MockMerkleIndex index;
	MockSkewCorrector corrector;

	MockIpSocket* mockSock = new MockIpSocket();
	std::shared_ptr<IIpSocket> sock(mockSock);
	peers._conn.reset(new PeerConnection(sock));

	Synchronizer sinkro(membership, peers, index, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "", mockSock->_history.calls() );
	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Leaf", "default" )
{
	MockMembership membership;
	MockPeerTracker peers;
	MockMerkleIndex index;
	MockSkewCorrector corrector;

	MockIpSocket* mockSock = new MockIpSocket();
	std::shared_ptr<IIpSocket> sock(mockSock);
	peers._conn.reset(new PeerConnection(sock));

	index._diff.push_back( whatsThePoint(10) );

	Synchronizer sinkro(membership, peers, index, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "", mockSock->_history.calls() );
	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "healKey(fooid none,10)", corrector._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Missing", "default" )
{
	MockMembership membership;
	MockPeerTracker peers;
	MockMerkleIndex index;
	MockSkewCorrector corrector;

	MockIpSocket* mockSock = new MockIpSocket();
	std::shared_ptr<IIpSocket> sock(mockSock);
	peers._conn.reset(new PeerConnection(sock));

	index._diff.push_back( whatsThePoint(3) );

	Synchronizer sinkro(membership, peers, index, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "", mockSock->_history.calls() );
	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "requestKeyRange(fooid none,3,3)", corrector._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Climb", "default" )
{
	MockMembership membership;
	MockPeerTracker peers;
	MockMerkleIndex index;
	MockSkewCorrector corrector;

	MockIpSocket* mockSock = new MockIpSocket();
	std::shared_ptr<IIpSocket> sock(mockSock);
	peers._conn.reset(new PeerConnection(sock));

	index._diff.push_back( whatsThePoint(1) );
	index._diff.push_back( whatsThePoint(2) );

	Synchronizer sinkro(membership, peers, index, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "send(merkle||1 1 10|2 2 20)", mockSock->_history.calls() );
	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "track(fooid)", peers._history.calls() );
	assertEquals( "", membership._history.calls() );
}

