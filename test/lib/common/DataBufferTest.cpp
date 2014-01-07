/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DataBuffer.h"
#include <iostream>
#include <string>

TEST_CASE( "DataBufferTest/testConstructors", "default" )
{
	{
		DataBuffer buff("0123456789", 10);
		assertEquals( "0123456789", buff.str() );
		assertEquals( "0123456789", std::string(buff.buffer(), buff.size()) );
		assertEquals( 10, buff.size() );
	}
}

TEST_CASE( "DataBufferTest/testSet", "default" )
{
	DataBuffer buff("0123456789", 10);
	assertEquals( "0123456789", buff.str() );

	DataBuffer other(DataBuffer::Null());
	other = buff;
	assertEquals( "0123456789", buff.str() );
}

TEST_CASE( "DataBufferTest/testRead", "default" )
{
	DataBuffer buff("0123456789", 10);
	assertEquals( "0123456789", buff.str() );
	assertEquals( "0123456789", std::string(buff.buffer(), buff.size()) );

	assertEquals( "012", buff.read(3) );
	assertEquals( "3456789", buff.str() );
	assertEquals( 7, buff.size() );

	assertEquals( "3456789", buff.read(200) );
	assertEquals( "", buff.str() );
	assertEquals( 0, buff.size() );
}

TEST_CASE( "DataBufferTest/testSkip", "default" )
{
	DataBuffer buffer("0123456789", 10);
	assertEquals( "0123456789", buffer.str() );

	buffer.skip(5);
	assertEquals( "56789", buffer.str() );
	assertEquals( 5, buffer.size() );

	buffer.skip(200);
	assertEquals( "", buffer.str() );
	assertEquals( 0, buffer.size() );
}

