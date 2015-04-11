/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Hash.h"
#include <string>
using std::string;

TEST_CASE( "HashTest/testDefault", "[unit]" )
{
	// TIGER192
	assertEquals("BFpzRnDYYAVmT1iRy7GsY_G2PZbAqrjm", Hash("hello").base64());
	assertEquals("rX8E2LSP5uTbtyZAtbES9MwMZI0kwAQs", Hash("world").base64());
}

TEST_CASE( "HashTest/testCompute", "[unit]" )
{
	string res1 = Hash("foo").base64();
	string res2 = Hash("foo").base64();
	assertEquals( res1, res2 );

	string hashed = Hash("foo").str();
	assertEquals( Hash().fromHash(hashed).str(), hashed );
}

TEST_CASE( "HashTest/testToFromBase64", "[unit]" )
{
	string base1 = Hash("foo").base64();
	string base2 = Hash().fromBase64(base1).base64();
	assertEquals( base1, base2 );

	assertEquals( Hash().fromBase64(base1).integer(), Hash().fromBase64(base2).integer() );
}
