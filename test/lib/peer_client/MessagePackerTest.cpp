/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MessagePacker.h"
#include <string>
using std::string;

TEST_CASE( "MessagePackerTest/testPackage", "[unit]" )
{
	MessagePacker packer;
	string header{0,6,42};
	assertEquals( (header + "hello"), packer.package(42, "hello") );

	string payload;
	payload.resize(257, 'a');
	header = {1, 2,(char)200}; // 256+2
	assertEquals( (header + payload), packer.package(200, payload.data(), payload.size()) );
}

TEST_CASE( "MessagePackerTest/testNull", "[unit]" )
{
	MessagePacker packer;
	string header{0,1,42};
	assertEquals( header, packer.package(42, NULL, 0) );
}

