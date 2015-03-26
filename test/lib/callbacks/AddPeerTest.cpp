/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AddPeer.h"

#include "api/WriteInstructions.h"
#include "common/KeyMetadata.h"
#include "common/turbopump_defaults.h"
#include "storage/readstream.h"
#include "storage/StringReader.h"

#include "mock/MockConsistentHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockKeyTabulator.h"
using std::string;

TEST_CASE( "AddPeerTest/testAdd", "[unit]" )
{
	MockConsistentHashRing ring;
	MockMembership membership;
	MockKeyTabulator index;
	AddPeer action(ring, membership, index);

	WriteInstructions params(MEMBERSHIP_FILE_PREFIX + string("fooid"), "v1", 0, 0);
	readstream contents( new StringReader("localhost:9001"), KeyMetadata() );
	assertTrue( action.run(params, contents) );

	assertEquals( "add(fooid)|addIp(localhost:9001,fooid)|save()", membership._history.calls() );
	assertEquals( "insert(fooid,fooid)", ring._history.calls() );
	assertEquals( "splitSection(fooid)", index._history.calls() );
}

TEST_CASE( "AddPeerTest/testAddExistingWorker", "[unit]" )
{
	MockConsistentHashRing ring;
	MockMembership membership;
	membership.add("fooid");
	membership._history.clear();
	MockKeyTabulator index;
	AddPeer action(ring, membership, index);

	WriteInstructions params(MEMBERSHIP_FILE_PREFIX + string("fooid"), "v1", 0, 0);
	readstream contents( new StringReader("localhost:9001"), KeyMetadata() );
	assertTrue( action.run(params, contents) );

	assertEquals( "add(fooid)|addIp(localhost:9001,fooid)|save()", membership._history.calls() );
	assertEquals( "", ring._history.calls() );
	assertEquals( "", index._history.calls() );
}