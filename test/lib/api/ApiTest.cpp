/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Api.h"
#include "ListKeys.h"

#include "common/DataBuffer.h"
#include "mock/MockDataStore.h"
#include "socket/StringByteStream.h"
#include <memory>

TEST_CASE( "ApiTest/testDefault", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	Api api(store, stream);

	std::unique_ptr<Op> op = api.op("list-keys");
	assertFalse( !op );

	assertTrue( op->run() );
	assertEquals( "report(0,.membership/)", store._history.calls() );

	/*Turbopump::Op op = api.write("filename", "version")
	op.run(bytes)
	op.run();*/
}

TEST_CASE( "ApiTest/testDeserializeFromBinary", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	Api api(store, stream);

	Turbopump::ListKeys params{true, true};
	msgpack::sbuffer sbuf;
	msgpack::pack(&sbuf, params);

	std::unique_ptr<Op> op = api.op("list-keys", DataBuffer(sbuf.data(), sbuf.size()));
	assertFalse( !op );

	assertTrue( op->run() );
	assertEquals( "report(1,)", store._history.calls() );
}

TEST_CASE( "ApiTest/testDeserializeFromMap", "[unit]" )
{
	MockDataStore store;
	StringByteStream stream;
	Api api(store, stream);

	std::unordered_map<std::string,std::string> params;
	params["all"] = "1";
	params["deleted"] = "1";

	std::unique_ptr<Op> op = api.op("list-keys", params);
	assertFalse( !op );

	assertTrue( op->run() );
	assertEquals( "report(1,)", store._history.calls() );
}

