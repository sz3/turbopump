/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Synchronizer.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockKeyTabulator.h"
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
	MockHashRing ring;
	MockMembership membership;
	membership.addIp("1.2.3.4", "dude");
	membership._history.clear();
	MockKeyTabulator index;
	index._tree._top = whatsThePoint(5);
	index._tree._id = TreeId("oak", 2);
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.pingRandomPeer();

	assertEquals( "locationsFromHash(oak,2)", ring._history.calls() );
	assertEquals( "randomPeer()", membership._history.calls() );
	assertEquals( "merklePing(dude,oak,5 5 50)", messenger._history.calls() );
	assertEquals( "randomTree()", index._history.calls() );
	assertEquals( "top()", index._tree._history.calls() );
}

TEST_CASE( "SynchronizerTest/testPingRandomHashRingLoc", "default" )
{
	MockHashRing ring;
	ring._workers.push_back("dude");
	MockMembership membership;
	membership.addIp("dude", "dude");
	membership._history.clear();
	MockKeyTabulator index;
	index._tree._top = whatsThePoint(5);
	index._tree._id = TreeId("oak", 2);
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.pingRandomPeer();

	assertEquals( "locationsFromHash(oak,2)", ring._history.calls() );
	assertEquals( "randomPeerFromList()", membership._history.calls() );
	assertEquals( "merklePing(dude,oak,5 5 50)", messenger._history.calls() );
	assertEquals( "randomTree()", index._history.calls() );
	assertEquals( "top()", index._tree._history.calls() );
}

TEST_CASE( "SynchronizerTest/testOffloadUnwantedKeys", "default" )
{
	MockHashRing ring;
	ring._workers.push_back("dude");
	MockMembership membership;
	membership.addIp("dude", "dude");
	membership._history.clear();
	MockKeyTabulator index;
	index._tree._top = whatsThePoint(5);
	index._tree._id = TreeId("oak", 2);
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.offloadUnwantedKeys();

	assertEquals( "unwantedTree()", index._history.calls() );
	assertEquals( "empty()", index._tree._history.calls() );
	assertEquals( "locationsFromHash(oak,2)", ring._history.calls() );
	assertEquals( "randomPeerFromList()|self()", membership._history.calls() );
	assertEquals( ("pushKeyRange(dude,oak,0," + StringUtil::str(~0ULL) + ",me)"), corrector._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.OtherSideEmpty", "default" )
{
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator index;
	index._tree._top = whatsThePoint(10);
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), TreeId("oak",4), MerklePoint::null());

	assertEquals( "find(oak,4)", index._history.calls() );
	assertEquals( "empty()", index._tree._history.calls() );
	assertEquals( ("pushKeyRange(fooid,oak,0," + StringUtil::str(~0ULL) + ",)"), corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.OurSideEmpty", "default" )
{
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	index._tree._diff.push_back( MerklePoint::null() );

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( ("requestKeyRange(fooid,oak,0," + StringUtil::str(~0ULL) + ")"), messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.BothSidesEmpty", "default" )
{
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator index;
	index._tree._empty = true;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), TreeId("oak"), MerklePoint::null());

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "empty()", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Same", "default" )
{
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.LeafDiff", "default" )
{
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	index._tree._diff.push_back( whatsThePoint(10) );

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "requestKeyRange(fooid,oak,10,18446744073709502218)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Missing", "default" )
{
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	index._tree._diff.push_back( whatsThePoint(32) );

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "requestKeyRange(fooid,oak,32,18446744069414584352)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Climb", "default" )
{
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;

	index._tree._diff.push_back( whatsThePoint(1) );
	index._tree._diff.push_back( whatsThePoint(2) );

	Synchronizer sinkro(ring, membership, index, messenger, corrector);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "merklePing(fooid,oak,1 1 10|2 2 20)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

