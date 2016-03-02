/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Api.h"
#include "Drop.h"
#include "Options.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockMessageSender.h"
#include "mock/MockSkewCorrector.h"
#include "mock/MockStatusReporter.h"
#include "mock/MockStore.h"
#include "mock/MockSynchronize.h"
#include "socket/NullByteStream.h"
#include <memory>

TEST_CASE( "ApiTest/testDefault", "[unit]" )
{
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;
	Turbopump::Options options;
	Turbopump::Api api(corrector, locator, messenger, reporter, store, sync, options);

	std::unique_ptr<Turbopump::Command> command = api.command("list-keys", std::unordered_map<std::string,std::string>());
	assertFalse( !command );

	NullByteStream stream;
	command->setWriter(&stream);

	assertTrue( command->run() );
	assertEquals( "enumerate(1000,)", store._history.calls() );
}

TEST_CASE( "ApiTest/testDeserializeFromBinary", "[unit]" )
{
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;
	Turbopump::Options options;
	Turbopump::Api api(corrector, locator, messenger, reporter, store, sync, options);

	locator._mine = false;
	store._reads["bomba"] = "yep";

	Turbopump::Drop params;
	params.copies = 2;
	params.name = "bomba";
	msgpack::sbuffer sbuf;
	msgpack::pack(&sbuf, params);

	std::unique_ptr<Turbopump::Command> command = api.command(Turbopump::Drop::_ID, sbuf.data(), sbuf.size());
	assertFalse( !command );

	NullByteStream stream;
	command->setWriter(&stream);

	assertTrue( command->run() );
	assertEquals( "read(bomba,)", store._history.calls() );
}

TEST_CASE( "ApiTest/testDeserializeFromBinary.BadId", "[unit]" )
{
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;
	Turbopump::Options options;
	Turbopump::Api api(corrector, locator, messenger, reporter, store, sync, options);

	locator._mine = false;
	store._reads["bomba"] = "yep";

	Turbopump::Drop params;
	params.copies = 2;
	params.name = "bomba";
	msgpack::sbuffer sbuf;
	msgpack::pack(&sbuf, params);

	std::unique_ptr<Turbopump::Command> command = api.command(250, sbuf.data(), sbuf.size());
	assertTrue( !command );
}

TEST_CASE( "ApiTest/testDeserializeFromBinary.DeserializeGarbage", "[unit]" )
{
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;
	Turbopump::Options options;
	Turbopump::Api api(corrector, locator, messenger, reporter, store, sync, options);

	locator._mine = false;
	store._reads["bomba"] = "yep";

	std::string garbage = "012345678hehehehe";
	std::unique_ptr<Turbopump::Command> command = api.command(Turbopump::Drop::_ID, garbage.data(), garbage.size());
	assertTrue( !command );
}

TEST_CASE( "ApiTest/testDeserializeFromMap", "[unit]" )
{
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;
	Turbopump::Options options;
	Turbopump::Api api(corrector, locator, messenger, reporter, store, sync, options);

	locator._mine = false;
	store._reads["bomba"] = "yep";

	std::unordered_map<std::string,std::string> params;
	params["copies"] = "5";
	params["name"] = "bomba";

	std::unique_ptr<Turbopump::Command> command = api.command("drop", params);
	assertFalse( !command );

	NullByteStream stream;
	command->setWriter(&stream);

	assertTrue( command->run() );
	assertEquals( "read(bomba,)", store._history.calls() );
}

TEST_CASE( "ApiTest/testFromRequest", "[unit]" )
{
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;
	Turbopump::Options options;
	Turbopump::Api api(corrector, locator, messenger, reporter, store, sync, options);

	locator._mine = false;
	store._reads["bomba"] = "yep";

	Turbopump::Drop req;
	req.copies = 2;
	req.name = "bomba";
	std::unique_ptr<Turbopump::Command> command = api.command(req);
	assertFalse( !command );

	NullByteStream stream;
	command->setWriter(&stream);

	assertTrue( command->run() );
	assertEquals( "read(bomba,)", store._history.calls() );
}
