/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AddPeer.h"

#include "common/KeyMetadata.h"
#include "common/turbopump_defaults.h"
#include "mock/MockDataStore.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockMerkleIndex.h"
using std::string;

TEST_CASE( "AddPeerTest/testAdd", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MockMerkleIndex index;
	AddPeer action(ring, membership, index);

	IDataStoreReader::ptr contents( new MockDataStore::Reader("localhost:9001") );
	assertTrue( action.run(KeyMetadata(MEMBERSHIP_FILE_PREFIX + string("fooid"), 0, 0), contents) );

	assertEquals( "add(fooid)|addIp(localhost:9001,fooid)|save()", membership._history.calls() );
	assertEquals( "addWorker(fooid)", ring._history.calls() );
	assertEquals( "splitSection(fooid)", index._history.calls() );
}

TEST_CASE( "AddPeerTest/testAddExistingWorker", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership.add("fooid");
	membership._history.clear();
	MockMerkleIndex index;
	AddPeer action(ring, membership, index);

	IDataStoreReader::ptr contents( new MockDataStore::Reader("localhost:9001") );
	assertTrue( action.run(KeyMetadata(MEMBERSHIP_FILE_PREFIX + string("fooid"), 0, 0), contents) );

	assertEquals( "add(fooid)|addIp(localhost:9001,fooid)|save()", membership._history.calls() );
	assertEquals( "", ring._history.calls() );
	assertEquals( "", index._history.calls() );
}
