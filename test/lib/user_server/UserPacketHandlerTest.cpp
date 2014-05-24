/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "UserPacketHandler.h"
#include "http/HttpByteStream.h"
#include "mock/MockDataStore.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockKeyTabulator.h"
#include "mock/MockProcessState.h"
#include "programmable/TurboApi.h"
#include "socket/StringByteStream.h"

TEST_CASE( "UserPacketHandlerTest/testDefault", "[unit]" )
{
	MockDataStore dataStore;
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator keyTabulator;
	MockProcessState state;
	TurboApi callbacks;

	state._summary = "dancing";

	{
		StringByteStream stream("GET /state HTTP/1.1\r\n\r\n");
		HttpByteStream httpStream(stream);
		UserPacketHandler board(httpStream, dataStore, ring, membership, keyTabulator, state, callbacks);
		board.run();

		assertEquals( "HTTP/1.1 200 Success\r\n"
					  "transfer-encoding: chunked\r\n\r\n"
					  "7\r\n"
					  "dancing\r\n"
					  "0\r\n\r\n", stream.writeBuffer() );
		assertEquals( "", stream.readBuffer() );
	}
}

TEST_CASE( "UserPacketHandlerTest/testQueryParam", "[unit]" )
{
	MockDataStore dataStore;
	MockHashRing ring;
	MockMembership membership;
	MockKeyTabulator keyTabulator;
	MockProcessState state;
	TurboApi callbacks;

	{
		StringByteStream stream("GET /local_list?deleted=true&all=true HTTP/1.1\r\n\r\n");
		HttpByteStream httpStream(stream);
		UserPacketHandler board(httpStream, dataStore, ring, membership, keyTabulator, state, callbacks);
		board.run();

		assertEquals( "report(1,)", dataStore._history.calls() );
		assertEquals( "HTTP/1.1 200 Success\r\n"
					  "transfer-encoding: chunked\r\n\r\n"
					  "0\r\n\r\n", stream.writeBuffer() );
		assertEquals( "", stream.readBuffer() );
	}
}

