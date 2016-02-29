/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Write.h"

#include "msgpack.hpp"
#include <string>
#include <unordered_map>
using std::string;
using std::unordered_map;

TEST_CASE( "WriteTest/testDefaults", "[unit]" )
{
	Turbopump::Write req;
	assertEquals( "", req.name );
	assertEquals( 0, req.mirror );
	assertEquals( 3, req.copies );
	assertEquals( 0, req.offset );
}

TEST_CASE( "WriteTest/testSerialization", "[unit]" )
{
	Turbopump::Write req;
	req.name = "foofile";
	req.version = "v1";
	req.mirror = 1;
	req.copies = 5;
	req.source = "what?";

	msgpack::sbuffer sbuf;
	msgpack::pack(&sbuf, req);

	Turbopump::Write actual;
	msgpack::unpacked msg;
	msgpack::unpack(&msg, sbuf.data(), sbuf.size());
	msg.get().convert(&actual);

	assertEquals( req.name, actual.name );
	assertEquals( req.version, actual.version );
	assertEquals( req.mirror, actual.mirror );
	assertEquals( req.copies, actual.copies );
	assertEquals( 0, actual.offset );
	assertEquals( req.source, actual.source );
}

TEST_CASE( "WriteTest/testLoadMap", "[unit]" )
{
	Turbopump::Write req;

	unordered_map<string,string> vals;
	vals["name"] = "foo bar";
	vals["version"] = "v2";
	vals["mirror"] = "2";
	vals["copies"] = "5";
	req.load(vals);

	assertEquals( "foo bar", req.name );
	assertEquals( "v2", req.version );
	assertEquals( 2, req.mirror );
	assertEquals( 5, req.copies );
	assertEquals( 0, req.offset );
}

