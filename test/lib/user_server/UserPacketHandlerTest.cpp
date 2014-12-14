/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "UserPacketHandler.h"
#include "api/Api.h"
#include "api/Options.h"
#include "http/HttpByteStream.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockMessageSender.h"
#include "mock/MockSkewCorrector.h"
#include "mock/MockStatusReporter.h"
#include "mock/MockStore.h"
#include "mock/MockSynchronize.h"
#include "socket/StringByteStream.h"

TEST_CASE( "UserPacketHandlerTest/testDefault", "[unit]" )
{
	Turbopump::Options options;
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;

	reporter._status = "dancing";

	{
		StringByteStream stream("GET /status HTTP/1.1\r\n\r\n");
		HttpByteStream httpStream(stream);
		Turbopump::Api api(corrector, locator, messenger, reporter, store, sync, options);
		UserPacketHandler handler(httpStream, api);
		handler.run();

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
	Turbopump::Options options;
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;

	{
		StringByteStream stream("GET /list-keys?deleted=1&all=1 HTTP/1.1\r\n\r\n");
		HttpByteStream httpStream(stream);
		Turbopump::Api api(corrector, locator, messenger, reporter, store, sync, options);
		UserPacketHandler handler(httpStream, api);
		handler.run();

		assertEquals( "report(1,)", store._history.calls() );
		assertEquals( "HTTP/1.1 200 Success\r\n"
					  "transfer-encoding: chunked\r\n\r\n"
					  "0\r\n\r\n", stream.writeBuffer() );
		assertEquals( "", stream.readBuffer() );
	}
}

