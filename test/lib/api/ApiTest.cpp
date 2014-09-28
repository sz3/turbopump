/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Api.h"
#include "Options.h"
#include "ListKeys.h"

#include "common/DataBuffer.h"
#include "mock/MockDataStore.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockMessageSender.h"
#include "mock/MockSkewCorrector.h"
#include "mock/MockStatusReporter.h"
#include "mock/MockSynchronize.h"
#include "socket/StringByteStream.h"
#include <memory>

TEST_CASE( "ApiTest/testDefault", "[unit]" )
{
	MockSkewCorrector corrector;
	MockDataStore store;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockSynchronize sync;
	StringByteStream stream;
	Turbopump::Options options;
	Turbopump::Api api(corrector, store, locator, messenger, reporter, sync, stream, options);

	Turbopump::ListKeys req;
	std::unique_ptr<Turbopump::Command> command = api.command("list-keys", std::unordered_map<std::string,std::string>());
	assertFalse( !command );

	assertTrue( command->run() );
	assertEquals( "report(0,.membership/)", store._history.calls() );
}

TEST_CASE( "ApiTest/testDeserializeFromBinary", "[unit]" )
{
	MockSkewCorrector corrector;
	MockDataStore store;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockSynchronize sync;
	StringByteStream stream;
	Turbopump::Options options;
	Turbopump::Api api(corrector, store, locator, messenger, reporter, sync, stream, options);

	Turbopump::ListKeys params;
	params.all = true;
	params.deleted = true;
	msgpack::sbuffer sbuf;
	msgpack::pack(&sbuf, params);

	std::unique_ptr<Turbopump::Command> command = api.command(Turbopump::ListKeys::_ID, DataBuffer(sbuf.data(), sbuf.size()));
	assertFalse( !command );

	assertTrue( command->run() );
	assertEquals( "report(1,)", store._history.calls() );
}

TEST_CASE( "ApiTest/testDeserializeFromMap", "[unit]" )
{
	MockSkewCorrector corrector;
	MockDataStore store;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockSynchronize sync;
	StringByteStream stream;
	Turbopump::Options options;
	Turbopump::Api api(corrector, store, locator, messenger, reporter, sync, stream, options);

	std::unordered_map<std::string,std::string> params;
	params["all"] = "1";
	params["deleted"] = "1";

	std::unique_ptr<Turbopump::Command> command = api.command("list-keys", params);
	assertFalse( !command );

	assertTrue( command->run() );
	assertEquals( "report(1,)", store._history.calls() );
}

TEST_CASE( "ApiTest/testFromRequest", "[unit]" )
{
	MockSkewCorrector corrector;
	MockDataStore store;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockSynchronize sync;
	StringByteStream stream;
	Turbopump::Options options;
	Turbopump::Api api(corrector, store, locator, messenger, reporter, sync, stream, options);

	Turbopump::ListKeys req;
	req.deleted = true;
	std::unique_ptr<Turbopump::Command> command = api.command(req);
	assertFalse( !command );

	assertTrue( command->run() );
	assertEquals( "report(1,.membership/)", store._history.calls() );
}
