/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Switchboard.h"
#include "mock/MockDataStore.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockKeyTabulator.h"
#include "mock/MockProcessState.h"
#include "programmable/TurboApi.h"
#include "socket/StringByteStream.h"

TEST_CASE( "SwitchboardTest/testDefault", "[unit]" )
{
	MockDataStore dataStore;
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator keyTabulator;
	MockProcessState state;
	TurboApi callbacks;

	state._summary = "dancing";

	{
		StringByteStream stream("state||");
		Switchboard board(stream, dataStore, ring, membership, keyTabulator, state, callbacks);
		board.run();

		assertEquals( "dancing", stream.writeBuffer() );
		assertEquals( "", stream.readBuffer() );
	}
}

