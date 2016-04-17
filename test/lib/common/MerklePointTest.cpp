/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MerklePoint.h"

TEST_CASE( "MerklePointTest/testToString", "[unit]" )
{
	MerklePoint point(65, 35, 42);
	assertEquals("35 42 65", MerklePointSerializer::toString(point));
}

TEST_CASE( "MerklePointTest/testFromString", "[unit]" )
{
	MerklePoint point;
	assertTrue( MerklePointSerializer::fromString(point, "1337 42 12345") );

	assertEquals( 1337, point.key );
	assertEquals( 42, point.keybits );
	assertEquals( 12345, point.hash );
}

TEST_CASE( "MerklePointTest/testSerialize", "[unit]" )
{
	MerklePoint point;
	point.key = 0x1234567812345678;
	point.keybits = 0x1234;
	point.hash = 0xFFFF123412345678;

	MerklePoint another;
	assertTrue( MerklePointSerializer::fromString(another, MerklePointSerializer::toString(point)) );

	assertEquals( 0x1234567812345678, another.key );
	assertEquals( 0x1234, another.keybits );
	assertEquals( 0xFFFF123412345678, another.hash );
}

