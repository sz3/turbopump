/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AckWriteCommand.h"

#include "Drop.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockSkewCorrector.h"
#include "mock/MockStore.h"

TEST_CASE( "AckWriteCommandTest/testAck.FileNotFound", "[unit]" )
{
	MockSkewCorrector corrector;
	MockStore store;
	MockLocateKeys locator;
	AckWriteCommand command(corrector, store, locator);

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 1234;

	assertFalse( command.run() );
	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	assertEquals( "", locator._history.calls() );
	assertEquals( "", corrector._history.calls() );
}

TEST_CASE( "AckWriteCommandTest/testAck.MismatchSize", "[unit]" )
{
	MockSkewCorrector corrector;
	MockStore store;
	store._reads["myfile"] = "foobar";
	MockLocateKeys locator;
	AckWriteCommand command(corrector, store, locator);

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 1234;

	assertFalse( command.run() );
	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	assertEquals( "", locator._history.calls() );
	assertEquals( "", corrector._history.calls() );
}

TEST_CASE( "AckWriteCommandTest/testAck.FileIsMine", "[unit]" )
{
	MockSkewCorrector corrector;
	MockStore store;
	store._reads["myfile"] = "foobar";
	MockLocateKeys locator;
	locator._mine = true;
	AckWriteCommand command(corrector, store, locator);

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 6;

	assertTrue( command.run() );
	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	assertEquals( "keyIsMine(myfile,3)", locator._history.calls() );
	assertEquals( "", corrector._history.calls() );
}

TEST_CASE( "AckWriteCommandTest/testAck.FileIsntMine", "[unit]" )
{
	MockSkewCorrector corrector;
	MockStore store;
	store._reads["myfile"] = "foobar";
	MockLocateKeys locator;
	locator._mine = false;
	AckWriteCommand command(corrector, store, locator);

	command.params.name = "myfile";
	command.params.version = "1,hi:1";
	command.params.size = 6;

	assertTrue( command.run() );
	assertEquals( "read(myfile,1,hi:1)", store._history.calls() );
	assertEquals( "keyIsMine(myfile,3)", locator._history.calls() );
	assertEquals( "dropKey(myfile)", corrector._history.calls() );
}
