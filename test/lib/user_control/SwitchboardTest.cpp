#include "unittest.h"

#include "Switchboard.h"
#include "mock/MockDataStore.h"
#include "mock/MockMembership.h"
#include "programmable/Callbacks.h"
#include "socket/StringByteStream.h"

TEST_CASE( "SwitchboardTest/testDefault", "[unit]" )
{
	MockDataStore dataStore;
	MockMembership membership;
	Callbacks callbacks;

	dataStore._store["hi"] = "world";

	{
		StringByteStream stream("local_list||");
		Switchboard board(stream, dataStore, membership, callbacks);
		board.run();

		assertEquals( "(hi)=>world", stream.writeBuffer() );
		assertEquals( "", stream.readBuffer() );
	}


}

