/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "RemovePeerCommand.h"
#include "common/WallClock.h"
#include "mock/DummyTurbopumpApi.h"
#include "mock/MockStoreWriter.h"

TEST_CASE( "RemovePeerCommandTest/testRemove", "[unit]" )
{
	WallClock().freeze(WallClock::MAGIC_NUMBER);

	DummyTurbopumpApi api;
	api.store._writer = new MockStoreWriter();
	RemovePeerCommand command(api);

	command.params.uid = "fooid";

	assertMsg( command.run(), command.status().str() );
	assertEquals( "write(.membership/fooid,2,delete.UNIXSECONDS=,fooid.UNIXSECONDS=,0)", api.store._history.calls() );
	assertEquals( "flush()|close()|reader()", MockStoreWriter::calls() );
}
