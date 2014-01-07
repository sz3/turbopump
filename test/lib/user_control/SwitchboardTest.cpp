/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Switchboard.h"
#include "mock/MockDataStore.h"
#include "mock/MockMembership.h"
#include "programmable/TurboApi.h"
#include "socket/StringByteStream.h"

TEST_CASE( "SwitchboardTest/testDefault", "[unit]" )
{
	MockDataStore dataStore;
	MockMembership membership;
	TurboApi callbacks;

	dataStore._store["hi"] = "world";

	{
		StringByteStream stream("local_list||");
		Switchboard board(stream, dataStore, membership, callbacks);
		board.run();

		assertEquals( "(hi)=>world", stream.writeBuffer() );
		assertEquals( "", stream.readBuffer() );
	}


}

