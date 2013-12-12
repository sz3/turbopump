#include "unittest.h"

#include "Synchronizer.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"
#include "mock/MockIpSocket.h"
#include "mock/MockMembership.h"
#include "mock/MockMerkleIndex.h"
#include "mock/MockMessageSender.h"
#include "mock/MockSkewCorrector.h"
#include "wan_server/PeerConnection.h"

#include "serialize/StringUtil.h"
#include "socket/IpAddress.h"

namespace {
	MerklePoint whatsThePoint(unsigned key, unsigned short keybits=0)
	{
		MerklePoint point;
		point.location.key = key;
		point.location.keybits = (keybits == 0)? key : keybits;
		point.hash = key * 10;
		return point;
	}
}

TEST_CASE( "SynchronizerTest/testPingRandomPeer", "default" )
{
	MockMembership membership;
	membership.addIp("1.2.3.4", "dude");
	MockMerkleIndex index;
	index._top = whatsThePoint(5);
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(membership, index, messenger, corrector);
	sinkro.pingRandomPeer();

	assertEquals( "addIp(1.2.3.4,dude)|randomPeer()", membership._history.calls() );
	assertEquals( "merklePing(dude,5 5 50)", messenger._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.OtherSideEmpty", "default" )
{
	MockMembership membership;
	MockMerkleIndex index;
	index._top = whatsThePoint(10);
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), MerklePoint::null());

	assertEquals( "top()", index._history.calls() );
	assertEquals( ("pushKeyRange(fooid,0," + StringUtil::str(~0ULL) + ")"), corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.OurSideEmpty", "default" )
{
	MockMembership membership;
	MockMerkleIndex index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	index._diff.push_back( MerklePoint::null() );

	Synchronizer sinkro(membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( ("requestKeyRange(fooid,0," + StringUtil::str(~0ULL) + ")"), messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.BothSidesEmpty", "default" )
{
	MockMembership membership;
	MockMerkleIndex index;
	index._top = MerklePoint::null();
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), MerklePoint::null());

	assertEquals( "top()", index._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Same", "default" )
{
	MockMembership membership;
	MockMerkleIndex index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.LeafDiff", "default" )
{
	MockMembership membership;
	MockMerkleIndex index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	index._diff.push_back( whatsThePoint(10) );

	Synchronizer sinkro(membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "requestKeyRange(fooid,10,18446744073709502218)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Missing", "default" )
{
	MockMembership membership;
	MockMerkleIndex index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	index._diff.push_back( whatsThePoint(32) );

	Synchronizer sinkro(membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "requestKeyRange(fooid,32,18446744069414584352)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Climb", "default" )
{
	MockMembership membership;
	MockMerkleIndex index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	index._diff.push_back( whatsThePoint(1) );
	index._diff.push_back( whatsThePoint(2) );

	Synchronizer sinkro(membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), whatsThePoint(10));

	assertEquals( "diff(10 10 100)", index._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "merklePing(fooid,1 1 10|2 2 20)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

