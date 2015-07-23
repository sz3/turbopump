/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Synchronizer.h"

#include "common/MerklePoint.h"
#include "membership/Peer.h"
#include "mock/MockConsistentHashRing.h"
#include "mock/MockKnownPeers.h"
#include "mock/MockKeyTabulator.h"
#include "mock/MockLogger.h"
#include "mock/MockMessageSender.h"
#include "mock/MockSkewCorrector.h"

#include "serialize/str.h"
using turbo::str::str;

namespace {
	MerklePoint whatsThePoint(unsigned key, unsigned short keybits=0)
	{
		MerklePoint point;
		point.location.key = key;
		point.location.keybits = (keybits == 0)? key : keybits;
		point.hash = key * 10;
		return point;
	}

	class TestableSynchronizer : public Synchronizer
	{
	public:
		using Synchronizer::Synchronizer;
		using Synchronizer::randomPeerFromList;
	};
}

TEST_CASE( "SynchronizerTest/testRandomPeerFromList", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	membership.update("dude");
	membership.update("abides");
	membership._history.clear();
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	TestableSynchronizer sinkro(ring, membership, index, messenger, corrector, logger);

	std::vector<std::string> list;
	assertFalse( sinkro.randomPeerFromList(list) );

	list.push_back("nope");
	assertFalse( sinkro.randomPeerFromList(list) );
	assertEquals( "lookup(nope)", membership._history.calls() );
	membership._history.clear();

	list.push_back("dude");
	std::shared_ptr<Peer> peer = sinkro.randomPeerFromList(list);
	assertEquals( "dude", peer->uid );
}

TEST_CASE( "SynchronizerTest/testPingRandomPeer", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	membership.update("dude");
	membership._history.clear();
	MockKeyTabulator index;
	index._tree._top = whatsThePoint(5);
	index._tree._id = TreeId("oak", 2);
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.pingRandomPeer();

	assertEquals( "locations(Rq5f,2)", ring._history.calls() );
	assertEquals( "randomPeer()", membership._history.calls() );
	assertEquals( "digestPing(dude,oak,5 5 50)", messenger._history.calls() );
	assertEquals( "randomTree()", index._history.calls() );
	assertEquals( "top()", index._tree._history.calls() );
}

TEST_CASE( "SynchronizerTest/testPingRandomHashRingLoc", "default" )
{
	MockConsistentHashRing ring;
	ring._locations.push_back("dude");
	MockKnownPeers membership;
	membership.update("dude");
	membership._history.clear();
	MockKeyTabulator index;
	index._tree._top = whatsThePoint(5);
	index._tree._id = TreeId("oak", 2);
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.pingRandomPeer();

	assertEquals( "locations(Rq5f,2)", ring._history.calls() );
	assertEquals( "lookup(dude)", membership._history.calls() );
	assertEquals( "digestPing(dude,oak,5 5 50)", messenger._history.calls() );
	assertEquals( "randomTree()", index._history.calls() );
	assertEquals( "top()", index._tree._history.calls() );
}

TEST_CASE( "SynchronizerTest/testOffloadUnwantedKeys", "default" )
{
	MockConsistentHashRing ring;
	ring._locations.push_back("dude");
	MockKnownPeers membership;
	membership.update("dude");
	membership._history.clear();
	MockKeyTabulator index;
	index._tree._top = whatsThePoint(5);
	index._tree._id = TreeId("oak", 2);
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.offloadUnwantedKeys();

	assertEquals( "unwantedTree()", index._history.calls() );
	assertEquals( "empty()", index._tree._history.calls() );
	assertEquals( "locations(Rq5f,2)", ring._history.calls() );
	assertEquals( "lookup(dude)|self()", membership._history.calls() );
	assertEquals( ("pushKeyRange(dude,oak,2,0," + str(~0ULL) + ",me)"), corrector._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.OtherSideEmpty", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	index._tree._top = whatsThePoint(10);
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak",4), MerklePoint::null());

	assertEquals( "find(oak,4)", index._history.calls() );
	assertEquals( "empty()", index._tree._history.calls() );
	assertEquals( ("pushKeyRange(fooid,oak,4,0," + str(~0ULL) + ",)"), corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.OurSideEmpty", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	index._tree._diff.push_back( MerklePoint::null() );

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( ("requestKeyRange(fooid,oak,0," + str(~0ULL) + ")"), messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.BothSidesEmpty", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	index._tree._empty = true;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), MerklePoint::null());

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "empty()", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Same", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.LeafDiff", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	index._tree._diff.push_back( whatsThePoint(10) );

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "requestKeyRange(fooid,oak,10,18446744073709502218)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.LeafDiffSameKey", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	index._tree._diff.push_back( whatsThePoint(64) );

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(64));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(64 64 640)", index._tree._history.calls() );
	assertEquals( "pushKey(fooid,oak,64)", corrector._history.calls() );
	assertEquals( "requestKey(fooid,oak,64)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.LeafDiffSameKey.Almost", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	index._tree._diff.push_back( whatsThePoint(64) );

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(64, 5));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(64 5 640)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "requestKeyRange(fooid,oak,64,18446744073709551431)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Missing", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	index._tree._diff.push_back( whatsThePoint(32) );

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "requestKeyRange(fooid,oak,32,18446744069414584352)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Climb", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	index._tree._diff.push_back( whatsThePoint(1) );
	index._tree._diff.push_back( whatsThePoint(2) );

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10));

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "digestPing(fooid,oak,1 1 10|2 2 20)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "SynchronizerTest/testCompare.Climb.isSyncResponse", "default" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	MockMessageSender messenger;
	MockSkewCorrector corrector;
	MockLogger logger;

	index._tree._diff.push_back( whatsThePoint(1) );
	index._tree._diff.push_back( whatsThePoint(2) );
	index._tree._diff.push_back( whatsThePoint(3) ); // should be discarded

	Synchronizer sinkro(ring, membership, index, messenger, corrector, logger);
	sinkro.compare(Peer("fooid"), TreeId("oak"), whatsThePoint(10), true);

	assertEquals( "find(oak,3)", index._history.calls() );
	assertEquals( "diff(10 10 100)", index._tree._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( "digestPing(fooid,oak,1 1 10|2 2 20)", messenger._history.calls() );
	assertEquals( "", membership._history.calls() );
}

