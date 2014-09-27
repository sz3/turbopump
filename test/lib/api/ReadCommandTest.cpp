/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ReadCommand.h"
#include "mock/MockDataStore.h"
#include "socket/StringByteStream.h"

TEST_CASE( "ReadCommandTest/testRead", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foo";
	StringByteStream stream;

	ReadCommand command(store, stream);
	command.params.name = "myfile";
	assertTrue( command.run() );

	assertEquals( "read(myfile)", store._history.calls() );
	assertEquals( "foo", stream.writeBuffer() );
}

TEST_CASE( "ReadCommandTest/testReadSpecificVersion", "[unit]" )
{
	MockDataStore store;
	store._store["myfile"] = "foo";
	StringByteStream stream;

	ReadCommand command(store, stream);
	command.params.name = "myfile";
	command.params.version = "v2";
	assertTrue( command.run() );

	assertEquals( "read(myfile,v2)", store._history.calls() );
	assertEquals( "foo", stream.writeBuffer() );
}

TEST_CASE( "ReadCommandTest/testReadNothing", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;

	ReadCommand command(store, stream);
	command.params.name = "myfile";
	assertFalse( command.run() );

	assertEquals( "read(myfile)", store._history.calls() );
	assertEquals( "", stream.writeBuffer() );
}
