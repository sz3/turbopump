/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ModifyPeer.h"

#include "api/WriteInstructions.h"
#include "common/KeyMetadata.h"
#include "common/turbopump_defaults.h"
#include "common/VectorClock.h"
#include "membership/Peer.h"
#include "storage/readstream.h"
#include "storage/StringReader.h"

#include "mock/MockConsistentHashRing.h"
#include "mock/MockKnownPeers.h"
#include "mock/MockKeyTabulator.h"
using std::string;

TEST_CASE( "ModifyPeerTest/testAdd", "[unit]" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	MockKeyTabulator index;
	ModifyPeer action(ring, membership, index);

	WriteInstructions params(MEMBERSHIP_FILE_PREFIX + string("fooid"), "v1", 0, 0);
	readstream contents( new StringReader("localhost:9001"), KeyMetadata() );
	assertTrue( action.run(params, contents) );

	assertEquals( "update(fooid,localhost:9001)|save()", membership._history.calls() );
	assertEquals( "insert(fooid,fooid)", ring._history.calls() );
	assertEquals( "splitSection(fooid)", index._history.calls() );
}

TEST_CASE( "ModifyPeerTest/testAddExistingWorker", "[unit]" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	membership.update("fooid");
	membership._history.clear();
	MockKeyTabulator index;
	ModifyPeer action(ring, membership, index);

	WriteInstructions params(MEMBERSHIP_FILE_PREFIX + string("fooid"), "v1", 0, 0);
	readstream contents( new StringReader("localhost:9001"), KeyMetadata() );
	assertTrue( action.run(params, contents) );

	assertEquals( "update(fooid,localhost:9001)|save()", membership._history.calls() );
	assertEquals( "", ring._history.calls() );
	assertEquals( "", index._history.calls() );
}

TEST_CASE( "ModifyPeerTest/testRemove", "[unit]" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	membership.update("fooid");
	membership._history.clear();
	MockKeyTabulator index;
	ModifyPeer action(ring, membership, index);

	VectorClock version;
	version.markDeleted();

	WriteInstructions params(MEMBERSHIP_FILE_PREFIX + string("fooid"), version.toString(), 0, 0);
	readstream contents( new StringReader(""), KeyMetadata() );
	assertTrue( action.run(params, contents) );

	assertEquals( "remove(fooid)|save()", membership._history.calls() );
	assertEquals( "erase(fooid)", ring._history.calls() );
	assertEquals( "cannibalizeSection(fooid)", index._history.calls() );
}
