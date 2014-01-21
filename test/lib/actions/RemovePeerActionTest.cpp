/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "RemovePeerAction.h"

#include "common/DataBuffer.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockMerkleIndex.h"
using std::map;
using std::string;

TEST_CASE( "RemovePeerActionTest/testRemove", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership.add("fooid");
	membership._history.clear();
	MockMerkleIndex index;
	RemovePeerAction action(ring, membership, index);

	map<string,string> params;
	params["uid"] = "fooid";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "remove(fooid)|save()", membership._history.calls() );
	assertEquals( "removeWorker(fooid)", ring._history.calls() );
	assertEquals( "cannibalizeTree(fooid)", index._history.calls() );
}

TEST_CASE( "RemovePeerActionTest/testRemoveNonexistentWorker", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership._history.clear();
	MockMerkleIndex index;
	RemovePeerAction action(ring, membership, index);

	map<string,string> params;
	params["uid"] = "nobody";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "remove(nobody)", membership._history.calls() );
	assertEquals( "", ring._history.calls() );
	assertEquals( "", index._history.calls() );
}
