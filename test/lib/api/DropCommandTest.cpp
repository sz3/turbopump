/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DropCommand.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockSkewCorrector.h"
#include "mock/MockStore.h"

TEST_CASE( "DropCommandTest/testDrop", "[unit]" )
{
	MockSkewCorrector corrector;
	MockStore store;
	store._reads["mine"] = "foo";

	DropCommand command(corrector, store);
	command.params.name = "mine";
	assertTrue( command.run() );

	assertEquals( "read(mine,)", store._history.calls() );
	assertEquals( "dropKey(mine)", corrector._history.calls() );
	assertEquals( 200, command.status() );
}
