/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DropCommand.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockStore.h"

TEST_CASE( "DropCommandTest/testKeyIsMine", "[unit]" )
{
	MockStore store;
	store._reads["mine"] = "foo";
	MockLocateKeys locator;
	locator._mine = true;

	DropCommand command(store, locator, NULL);
	command.params.name = "mine";
	assertFalse( command.run() );

	assertEquals( "read(mine,)", store._history.calls() );
	assertEquals( "keyIsMine(mine,3)", locator._history.calls() );
	assertEquals( 400, command.status() );
}

TEST_CASE( "DropCommandTest/testKeyIsntMine", "[unit]" )
{
	MockStore store;
	store._reads["notmine"] = "foo";
	MockLocateKeys locator;
	locator._mine = false;

	DropCommand command(store, locator, NULL);
	command.params.name = "notmine";

	assertTrue( command.run() );

	assertEquals( "read(notmine,)|remove(notmine)", store._history.calls() );
	assertEquals( "keyIsMine(notmine,3)", locator._history.calls() );
	assertEquals( 200, command.status() );
}

TEST_CASE( "DropCommandTest/testCallback", "[unit]" )
{
	CallHistory history;

	MockStore store;
	store._reads["key"] = "foo";
	MockLocateKeys locator;
	locator._mine = false;

	DropCommand command(store, locator, [&](Turbopump::Drop md){ history.call("onDrop", md.name, md.copies); });
	command.params.name = "key";
	assertTrue( command.run() );

	assertEquals( "read(key,)|remove(key)", store._history.calls() );
	assertEquals( "keyIsMine(key,3)", locator._history.calls() );
	assertEquals( "onDrop(key,3)", history.calls() );
}
