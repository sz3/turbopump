/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DropCommand.h"
#include "common/DataBuffer.h"
#include "mock/MockDataStore.h"
#include "mock/MockLocateKeys.h"

TEST_CASE( "DropCommandTest/testKeyIsMine", "[unit]" )
{
	MockDataStore store;
	store._store["mine"] = "foo";
	MockLocateKeys locator;
	locator._mine = true;

	DropCommand command(store, locator, NULL);
	command.params.name = "mine";
	assertFalse( command.run(DataBuffer::Null()) );

	assertEquals( "read(mine)", store._history.calls() );
	assertEquals( "foo", store._store["mine"] );
	assertEquals( "keyIsMine(mine,1)", locator._history.calls() );
}

TEST_CASE( "DropCommandTest/testKeyIsntMine", "[unit]" )
{
	MockDataStore store;
	store._store["notmine"] = "foo";
	MockLocateKeys locator;
	locator._mine = false;

	DropCommand command(store, locator, NULL);
	command.params.name = "notmine";

	assertTrue( command.run(DataBuffer::Null()) );

	assertEquals( "read(notmine)|drop(notmine)", store._history.calls() );
	assertEquals( "", store._store["notmine"] );
	assertEquals( "keyIsMine(notmine,1)", locator._history.calls() );
}

TEST_CASE( "DropCommandTest/testCallback", "[unit]" )
{
	CallHistory history;

	MockDataStore store;
	store._store["key"] = "foo";
	MockLocateKeys locator;
	locator._mine = false;

	DropCommand command(store, locator, [&](Turbopump::Drop md){ history.call("onDrop", md.name, md.copies); });
	command.params.name = "key";
	assertTrue( command.run(DataBuffer::Null()) );

	assertEquals( "read(key)|drop(key)", store._history.calls() );
	assertEquals( "", store._store["key"] );
	assertEquals( "keyIsMine(key,1)", locator._history.calls() );
	assertEquals( "onDrop(key,1)", history.calls() );
}
