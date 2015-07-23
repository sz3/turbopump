/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DropCommand.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockSkewCorrector.h"
#include "mock/MockStore.h"

TEST_CASE( "DropCommandTest/testKeyIsMine", "[unit]" )
{
	MockSkewCorrector corrector;
	MockStore store;
	store._reads["mine"] = "foo";
	MockLocateKeys locator;
	locator._mine = true;

	DropCommand command(corrector, store, locator);
	command.params.name = "mine";
	assertFalse( command.run() );

	assertEquals( "read(mine,)", store._history.calls() );
	assertEquals( "keyIsMine(mine,3)", locator._history.calls() );
	assertEquals( "", corrector._history.calls() );
	assertEquals( 400, command.status() );
}

TEST_CASE( "DropCommandTest/testKeyIsntMine", "[unit]" )
{
	MockSkewCorrector corrector;
	MockStore store;
	store._reads["notmine"] = "foo";
	MockLocateKeys locator;
	locator._mine = false;

	DropCommand command(corrector, store, locator);
	command.params.name = "notmine";

	assertTrue( command.run() );

	assertEquals( "read(notmine,)", store._history.calls() );
	assertEquals( "keyIsMine(notmine,3)", locator._history.calls() );
	assertEquals( "dropKey(notmine)", corrector._history.calls() );
	assertEquals( 200, command.status() );
}
