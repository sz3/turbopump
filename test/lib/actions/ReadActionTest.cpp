/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ReadAction.h"

#include "common/DataBuffer.h"
#include "mock/MockDataStore.h"
#include "socket/StringByteStream.h"
using std::map;
using std::string;

TEST_CASE( "ReadActionTest/testRead", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foo";
	StringByteStream stream;

	ReadAction action(store, stream);

	map<string,string> params;
	params["name"] = "myfile";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile)", store._history.calls() );
	assertEquals( "foo", stream.writeBuffer() );
}

TEST_CASE( "ReadActionTest/testReadSpecificVersion", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foo";
	StringByteStream stream;

	ReadAction action(store, stream);

	map<string,string> params;
	params["name"] = "myfile";
	params["version"] = "v2";
	action.setParams(params);

	assertTrue( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile,v2)", store._history.calls() );
	assertEquals( "foo", stream.writeBuffer() );
}

TEST_CASE( "ReadActionTest/testReadNothing", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	ReadAction action(store, stream);

	map<string,string> params;
	params["name"] = "myfile";
	action.setParams(params);

	assertFalse( action.run(DataBuffer::Null()) );

	assertEquals( "read(myfile)", store._history.calls() );
	assertEquals( "", stream.writeBuffer() );
}

