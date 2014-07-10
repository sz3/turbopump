/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AckWriteAction.h"
#include "actions/DropParams.h"
#include "common/DataBuffer.h"
#include "common/VectorClock.h"
#include "mock/MockDataStore.h"
#include "mock/MockLocateKeys.h"
using std::map;
using std::string;

TEST_CASE( "AckWriteActionTest/testAck.FileNotFound", "default" )
{
	MockDataStore store;
	MockLocateKeys locator;
	AckWriteAction action(store, locator);

	map<string,string> params;
	params["name"] = "myfile";
	params["v"] = "1,hi:1";
	params["size"] = "1234";
	action.setParams(params);

	assertFalse( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	assertEquals( "", locator._history.calls() );
}

TEST_CASE( "AckWriteActionTest/testAck.MismatchSize", "default" )
{
	MockDataStore store;
	store._store["myfile"] = "foobar";
	MockLocateKeys locator;
	AckWriteAction action(store, locator);

	map<string,string> params;
	params["name"] = "myfile";
	params["v"] = "1,hi:1";
	params["size"] = "1234";
	action.setParams(params);

	assertFalse( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	assertEquals( "foobar", store._store["myfile"] );
	assertEquals( "", locator._history.calls() );
}

TEST_CASE( "AckWriteActionTest/testAck.FileIsMine", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foobar";
	MockLocateKeys locator;
	locator._mine = true;
	AckWriteAction action(store, locator);

	map<string,string> params;
	params["name"] = "myfile";
	params["v"] = "1,hi:1";
	params["size"] = "6";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	assertEquals( "foobar", store._store["myfile"] );
	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
}

TEST_CASE( "AckWriteActionTest/testAck.FileIsntMine", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foobar";
	MockLocateKeys locator;
	locator._mine = false;
	AckWriteAction action(store, locator);

	map<string,string> params;
	params["name"] = "myfile";
	params["v"] = "1,hi:1";
	params["size"] = "6";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile,1,hi:1)|drop(myfile)", store._history.calls() );
	assertEquals( "", store._store["myfile"] );
	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
}

TEST_CASE( "AckWriteActionTest/testAck.DropCallback", "[unit]" )
{
	CallHistory history;

	MockDataStore store;
	store._store["myfile"] = "foobar";
	MockLocateKeys locator;
	locator._mine = false;
	AckWriteAction action(store, locator, [&](DropParams md){ history.call("onDrop", md.filename, md.totalCopies); });

	map<string,string> params;
	params["name"] = "myfile";
	params["v"] = "1,hi:1";
	params["size"] = "6";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile,1,hi:1)|drop(myfile)", store._history.calls() );
	assertEquals( "", store._store["myfile"] );
	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
	assertEquals( "onDrop(myfile,1)", history.calls() );
}
