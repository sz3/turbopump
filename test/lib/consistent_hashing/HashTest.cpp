/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "Hash.h"
#include <string>
using std::string;

TEST_CASE( "HashTest/testFromBytes", "[unit]" )
{
	string bytes;
	bytes.resize(16); //128 bits
	for (unsigned char i = 0; i < 16; ++i)
		bytes[i] = i;

	Hash hash(bytes);
	assertEquals( 0x0706050403020100, hash.integer() );

	string result = hash.bytes();
	for (int i = 0; i < 16; ++i)
		assertEquals( i, result[i] );
}

TEST_CASE( "HashTest/testCompute", "[unit]" )
{
	string res1 = Hash::compute("foo").bytes();
	string res2 = Hash::compute("foo").bytes();
	assertEquals( res1, res2 );
}

TEST_CASE( "HashTest/testToFromBase64", "[unit]" )
{
	string base1 = Hash::compute("foo").base64();
	string base2 = Hash::fromBase64(base1).base64();
	assertEquals( base1, base2 );

	assertEquals( Hash::fromBase64(base1).integer(), Hash::fromBase64(base2).integer() );
}
