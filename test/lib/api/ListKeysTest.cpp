/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ListKeys.h"

#include "msgpack.hpp"
#include <string>
#include <unordered_map>
using std::string;
using std::unordered_map;

TEST_CASE( "ListKeysTest/testDefaults", "[unit]" )
{
	Turbopump::ListKeys req;
	assertEquals( false, req.all );
	assertEquals( false, req.deleted );
}

TEST_CASE( "ListKeysTest/testSerialization", "[unit]" )
{
	Turbopump::ListKeys req;
	req.all = true;
	req.deleted = true;

	msgpack::sbuffer sbuf;
	msgpack::pack(&sbuf, req);

	Turbopump::ListKeys actual;
	msgpack::unpacked msg;
	msgpack::unpack(&msg, sbuf.data(), sbuf.size());
	msg.get().convert(&actual);

	assertEquals( req.all, actual.all );
	assertEquals( req.deleted, actual.deleted );
}

TEST_CASE( "ListKeysTest/testLoadMap", "[unit]" )
{
	Turbopump::ListKeys req;

	unordered_map<string,string> vals;
	vals["all"] = "1";
	vals["deleted"] = "0";
	req.load(vals);

	assertEquals( true, req.all );
	assertEquals( false, req.deleted );
}

