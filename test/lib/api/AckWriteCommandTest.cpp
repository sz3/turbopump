/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AckWriteCommand.h"

#include "Drop.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockStore.h"

TEST_CASE( "AckWriteCommandTest/testAck.FileNotFound", "[unit]" )
{
	MockStore store;
	MockLocateKeys locator;
	AckWriteCommand command(store, locator);

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 1234;

	assertFalse( command.run() );
	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	assertEquals( "", locator._history.calls() );
}

TEST_CASE( "AckWriteCommandTest/testAck.MismatchSize", "[unit]" )
{
	MockStore store;
	store._reads["myfile"] = "foobar";
	MockLocateKeys locator;
	AckWriteCommand command(store, locator);

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 1234;

	assertFalse( command.run() );
	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	//assertEquals( "foobar", store._store["myfile"] );
	assertEquals( "", locator._history.calls() );
}

TEST_CASE( "AckWriteCommandTest/testAck.FileIsMine", "[unit]" )
{
	MockStore store;
	store._reads["myfile"] = "foobar";
	MockLocateKeys locator;
	locator._mine = true;
	AckWriteCommand command(store, locator);

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 6;

	assertTrue( command.run() );
	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	//assertEquals( "foobar", store._store["myfile"] );
	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
}

TEST_CASE( "AckWriteCommandTest/testAck.FileIsntMine", "[unit]" )
{
	MockStore store;
	store._reads["myfile"] = "foobar";
	MockLocateKeys locator;
	locator._mine = false;
	AckWriteCommand command(store, locator);

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 6;

	assertTrue( command.run() );
	assertEquals( "read(myfile,1,hi:1)|drop(myfile)", store._history.calls() );
	//assertEquals( "", store._store["myfile"] );
	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
}

TEST_CASE( "AckWriteCommandTest/testAck.DropCallback", "[unit]" )
{
	CallHistory history;

	MockStore store;
	store._reads["myfile"] = "foobar";
	MockLocateKeys locator;
	locator._mine = false;
	AckWriteCommand command(store, locator, [&](const Turbopump::Drop& md){ history.call("onDrop", md.name, md.copies); });

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 6;

	assertTrue( command.run() );
	assertEquals( "read(myfile,1,hi:1)|drop(myfile)", store._history.calls() );
	//assertEquals( "", store._store["myfile"] );
	assertEquals( "keyIsMine(myfile,1)", locator._history.calls() );
	assertEquals( "onDrop(myfile,1)", history.calls() );
}
