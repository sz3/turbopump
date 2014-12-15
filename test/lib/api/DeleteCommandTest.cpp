/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DeleteCommand.h"
#include "common/VectorClock.h"
#include "mock/DummyTurbopumpApi.h"
#include "mock/MockStoreWriter.h"
using std::unique_ptr;

TEST_CASE( "DeleteCommandTest/testDelete", "[unit]" )
{
	DummyTurbopumpApi api;
	api.store._writer = new MockStoreWriter();
	DeleteCommand command(api);

	VectorClock version;
	version.increment("foo");
	command.params.version = version.toString();
	command.params.name = "deleted!";

	assertTrue( command.run() );
	assertEquals( "write(deleted!,2,delete:1,foo:1,0)", api.store._history.calls() );
	assertEquals( "write(timestamp)|flush()|reader()|close()", MockStoreWriter::calls() );
}
