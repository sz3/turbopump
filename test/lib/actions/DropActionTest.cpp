/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DropAction.h"
#include "actions/DropParams.h"
#include "common/DataBuffer.h"
#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
using std::map;
using std::string;

TEST_CASE( "DropActionTest/testFileIsMine", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foo";
	MockHashRing ring;
	ring._workers = {"aaa", "bbb", "ccc"};
	MockMembership membership;
	membership._self.reset( new Peer("aaa") );
	DropAction action(store, ring, membership);

	map<string,string> params;
	params["name"] = "myfile";
	action.setParams(params);

	assertFalse( action.run(DataBuffer::Null()) );

	assertEquals( "", store._history.calls() );
	assertEquals( "foo", store._store["myfile"] );

	assertEquals( "locations(myfile,1)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
}

TEST_CASE( "DropActionTest/testFileIsntMine", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foo";
	MockHashRing ring;
	ring._workers = {"aaa", "bbb", "ccc"};
	MockMembership membership;
	DropAction action(store, ring, membership);

	map<string,string> params;
	params["name"] = "myfile";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "erase(myfile)", store._history.calls() );
	assertEquals( "", store._store["myfile"] );

	assertEquals( "locations(myfile,1)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
}

TEST_CASE( "DropActionTest/testCallback", "[unit]" )
{
	CallHistory history;

	MockDataStore store;
	store._store["myfile"] = "foo";
	MockHashRing ring;
	ring._workers = {"aaa", "bbb", "ccc"};
	MockMembership membership;
	DropAction action(store, ring, membership, [&](DropParams md){ history.call("onDrop", md.filename, md.totalCopies); });

	map<string,string> params;
	params["name"] = "myfile";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "erase(myfile)", store._history.calls() );
	assertEquals( "", store._store["myfile"] );
	assertEquals( "locations(myfile,1)", ring._history.calls() );
	assertEquals( "onDrop(myfile,1)", history.calls() );
}
