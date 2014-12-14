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

	assertTrue( command.run() );
	assertEquals( "Writer::setOffset(0)|"
				  "Writer::write(localhost:9001)|"
				  "commit(.membership/fooid,{1,fooid:1},0)", api.store._history.calls() );
	assertEquals( "", MockStoreWriter().calls() );
}
