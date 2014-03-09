/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DropAction.h"
#include "actions/DropParams.h"
#include "common/DataBuffer.h"
#include "mock/MockDataStore.h"
#include "mock/MockLocateKeys.h"
using std::map;
using std::string;

TEST_CASE( "DropActionTest/testFileIsMine", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foo";
	MockLocateKeys locator;
	locator._mine = true;
	DropAction action(store, locator);

	map<string,string> params;
	params["name"] = "myfile";
	action.setParams(params);

	assertFalse( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile)", store._history.calls() );
	assertEquals( "foo", store._store["myfile"] );

	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
}

TEST_CASE( "DropActionTest/testFileIsntMine", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foo";
	MockLocateKeys locator;
	locator._mine = false;
	DropAction action(store, locator);

	map<string,string> params;
	params["name"] = "myfile";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile)|drop(myfile)", store._history.calls() );
	assertEquals( "", store._store["myfile"] );

	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
}

TEST_CASE( "DropActionTest/testCallback", "[unit]" )
{
	CallHistory history;

	MockDataStore store;
	store._store["myfile"] = "foo";
	MockLocateKeys locator;
	locator._mine = false;
	DropAction action(store, locator, [&](DropParams md){ history.call("onDrop", md.filename, md.totalCopies); });

	map<string,string> params;
	params["name"] = "myfile";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile)|drop(myfile)", store._history.calls() );
	assertEquals( "", store._store["myfile"] );
	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
	assertEquals( "onDrop(myfile,1)", history.calls() );
}
