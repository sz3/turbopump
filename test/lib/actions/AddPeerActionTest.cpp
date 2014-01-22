/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AddPeerAction.h"

#include "common/DataBuffer.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockMerkleIndex.h"
using std::map;
using std::string;

TEST_CASE( "AddPeerActionTest/testAdd", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MockMerkleIndex index;
	AddPeerAction action(ring, membership, index);

	map<string,string> params;
	params["uid"] = "fooid";
	params["ip"] = "localhost:9001";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "add(fooid)|addIp(localhost:9001,fooid)|save()", membership._history.calls() );
	assertEquals( "addWorker(fooid)", ring._history.calls() );
	assertEquals( "splitSection(fooid)", index._history.calls() );
}

TEST_CASE( "AddPeerActionTest/testAddExistingWorker", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership.add("fooid");
	membership._history.clear();
	MockMerkleIndex index;
	AddPeerAction action(ring, membership, index);

	map<string,string> params;
	params["uid"] = "fooid";
	params["ip"] = "localhost:9001";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "add(fooid)|addIp(localhost:9001,fooid)|save()", membership._history.calls() );
	assertEquals( "", ring._history.calls() );
	assertEquals( "", index._history.calls() );
}
