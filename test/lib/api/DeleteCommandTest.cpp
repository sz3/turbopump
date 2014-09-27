/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DeleteCommand.h"

#include "DummyTurbopumpApi.h"
#include "common/VectorClock.h"
using std::unique_ptr;

TEST_CASE( "DeleteCommandTest/testDelete", "[unit]" )
{
	DummyTurbopumpApi api;
	DeleteCommand command(api);

	VectorClock version;
	version.increment("foo");
	command.params.version = version.toString();
	command.params.name = "deleted!";

	assertTrue( command.run() );
	assertEquals( "timestamp", api.dataStore._store["deleted!"] );
	// TODO: need to get version out of mock.
}
