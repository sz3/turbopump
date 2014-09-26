/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Api.h"
#include "Options.h"
#include "ListKeys.h"

#include "common/DataBuffer.h"
#include "mock/MockDataStore.h"
#include "socket/StringByteStream.h"
#include <memory>

TEST_CASE( "ApiTest/testDefault", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	Turbopump::Options options;
	Turbopump::Api api(store, stream, options);

	std::unique_ptr<Turbopump::Command> command = api.command("list-keys");
	assertFalse( !command );

	assertTrue( command->run(DataBuffer::Null()) );
	assertEquals( "report(0,.membership/)", store._history.calls() );
}

TEST_CASE( "ApiTest/testDeserializeFromBinary", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	Turbopump::Options options;
	Turbopump::Api api(store, stream, options);

	Turbopump::ListKeys params;
	params.all = true;
	params.deleted = true;
	msgpack::sbuffer sbuf;
	msgpack::pack(&sbuf, params);

	std::unique_ptr<Turbopump::Command> command = api.command(Turbopump::ListKeys::ID, DataBuffer(sbuf.data(), sbuf.size()));
	assertFalse( !command );

	assertTrue( command->run(DataBuffer::Null()) );
	assertEquals( "report(1,)", store._history.calls() );
}

TEST_CASE( "ApiTest/testDeserializeFromMap", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	Turbopump::Options options;
	Turbopump::Api api(store, stream, options);

	std::unordered_map<std::string,std::string> params;
	params["all"] = "1";
	params["deleted"] = "1";

	std::unique_ptr<Turbopump::Command> command = api.command("list-keys", params);
	assertFalse( !command );

	assertTrue( command->run(DataBuffer::Null()) );
	assertEquals( "report(1,)", store._history.calls() );
}

