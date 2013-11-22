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
	assertEquals( 0x03020100, hash.sizet() );

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
