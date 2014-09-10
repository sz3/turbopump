/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Hash.h"
#include <string>
using std::string;

TEST_CASE( "HashTest/testDefault", "[unit]" )
{
	// TIGER192
	assertEquals("9DpzPnBWW8TmR/iPy5EsWZE0NXb8qrjm", Hash("hello").base64());
	assertEquals("rV6C0JQN3uRbtyX8tbCQ7KwKXG+kw8Os", Hash("world").base64());
}

TEST_CASE( "HashTest/testCompute", "[unit]" )
{
	string res1 = Hash("foo").base64();
	string res2 = Hash("foo").base64();
	assertEquals( res1, res2 );
}

TEST_CASE( "HashTest/testToFromBase64", "[unit]" )
{
	string base1 = Hash("foo").base64();
	string base2 = Hash().fromBase64(base1).base64();
	assertEquals( base1, base2 );

	assertEquals( Hash().fromBase64(base1).integer(), Hash().fromBase64(base2).integer() );
}
