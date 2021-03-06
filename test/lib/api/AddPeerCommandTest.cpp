/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AddPeerCommand.h"
#include "common/WallClock.h"
#include "mock/DummyTurbopumpApi.h"
#include "mock/MockStoreWriter.h"

TEST_CASE( "AddPeerCommandTest/testAdd", "[unit]" )
{
	WallClock().freeze(WallClock::MAGIC_NUMBER);

	DummyTurbopumpApi api;
	api.store._writer = new MockStoreWriter();
	AddPeerCommand command(api);

	command.params.uid = "fooid";
	command.params.ip = "localhost:9001";

	assertMsg( command.run(), command.status().str() );
	assertEquals( "write(.membership/fooid,1,fooid.UNIXSECONDS=,0)", api.store._history.calls() );
	assertEquals( "write(localhost:9001)|flush()|close()|reader()", MockStoreWriter::calls() );
}
