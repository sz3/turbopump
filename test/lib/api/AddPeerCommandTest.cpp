/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "AddPeerCommand.h"
#include "mock/DummyTurbopumpApi.h"
#include "mock/MockStoreWriter.h"

TEST_CASE( "AddPeerCommandTest/testAdd", "[unit]" )
{
	DummyTurbopumpApi api;
	api.store._writer = new MockStoreWriter();
	AddPeerCommand command(api);

	command.params.uid = "fooid";
	command.params.ip = "localhost:9001";

	assertMsg( command.run(), command.status().str() );
	assertEquals( "write(.membership/fooid,1,fooid:1,0)", api.store._history.calls() );
	assertEquals( "write(localhost:9001)|flush()|reader()|close()", MockStoreWriter::calls() );
}
