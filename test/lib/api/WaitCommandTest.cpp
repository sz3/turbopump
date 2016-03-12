/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WaitCommand.h"
#include "mock/MockStore.h"
#include "mock/MockWatches.h"
#include "socket/StringByteStream.h"

TEST_CASE( "WaitCommandTest/testNoNeedToWait", "[unit]" )
{
	MockStore store;
	store._reads["myfile"] = "foo";
	MockWatches watches;
	StringByteStream stream;

	WaitCommand command(store, watches);
	command.setWriter(&stream);
	command.params.name = "myfile";
	assertTrue( command.run() );

	assertEquals( "read(myfile,)", store._history.calls() );
	assertEquals( "watch(myfile)|unwatch(myfile)", watches._history.calls() );
	assertEquals( "myfile", stream.writeBuffer() );
	assertEquals( 200, command.status() );
}

TEST_CASE( "WaitCommandTest/testWait", "[unit]" )
{
	MockStore store;
	MockWatches watches;
	StringByteStream stream;

	WaitCommand command(store, watches);
	command.setWriter(&stream);
	command.params.name = "myfile";
	assertFalse( command.run() );

	assertEquals( "read(myfile,)|read(myfile,)", store._history.calls() );
	assertEquals( "watch(myfile)|unwatch(myfile)", watches._history.calls() );
	assertEquals( "", stream.writeBuffer() );
	assertEquals( 404, command.status() );
}

