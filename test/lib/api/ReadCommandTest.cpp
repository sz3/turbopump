/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ReadCommand.h"
#include "mock/MockStore.h"
#include "socket/StringByteStream.h"

TEST_CASE( "ReadCommandTest/testRead", "[unit]" )
{
	MockStore store;
	store._reads["myfile"] = "foo";
	StringByteStream stream;

	ReadCommand command(store);
	command.setWriter(&stream);
	command.params.name = "myfile";
	assertTrue( command.run() );

	assertEquals( "read(myfile,)", store._history.calls() );
	assertEquals( "foo", stream.writeBuffer() );
	assertEquals( 200, command.status() );
}

TEST_CASE( "ReadCommandTest/testNullWriter", "[unit]" )
{
	MockStore store;
	ReadCommand command(store);
	command.params.name = "myfile";
	assertFalse( command.run() );

	assertEquals( "", store._history.calls() );
	assertEquals( 0, command.status() );
}

TEST_CASE( "ReadCommandTest/testReadSpecificVersion", "[unit]" )
{
	MockStore store;
	store._reads["myfile"] = "foo";
	StringByteStream stream;

	ReadCommand command(store);
	command.setWriter(&stream);
	command.params.name = "myfile";
	command.params.version = "v2";
	assertTrue( command.run() );

	assertEquals( "read(myfile,v2)", store._history.calls() );
	assertEquals( "foo", stream.writeBuffer() );
	assertEquals( 200, command.status() );
}

TEST_CASE( "ReadCommandTest/testReadNothing", "[unit]" )
{
	MockStore store;
	StringByteStream stream;

	ReadCommand command(store);
	command.setWriter(&stream);
	command.params.name = "myfile";
	assertFalse( command.run() );

	assertEquals( "read(myfile,)", store._history.calls() );
	assertEquals( "", stream.writeBuffer() );
	assertEquals( 404, command.status() );
}
