#include "unittest.h"

#include "MerklePoint.h"

TEST_CASE( "MerklePointTest/testToString", "[unit]" )
{
	MerklePoint point;
	point.location = merkle_location<unsigned long long>(35, 42);
	point.hash = 65;

	assertEquals("35 42 65", MerklePointSerializer::toString(point));
}

TEST_CASE( "MerklePointTest/testFromString", "[unit]" )
{
	MerklePoint point;
	assertTrue( MerklePointSerializer::fromString(point, "1337 42 12345") );

	assertEquals( 1337, point.location.key );
	assertEquals( 42, point.location.keybits );
	assertEquals( 12345, point.hash );
}

TEST_CASE( "MerklePointTest/testSerialize", "[unit]" )
{
	MerklePoint point;
	point.location = merkle_location<unsigned long long>(0x1234567812345678, 0x1234);
	point.hash = 0xFFFF123412345678;

	MerklePoint another;
	assertTrue( MerklePointSerializer::fromString(another, MerklePointSerializer::toString(point)) );

	assertEquals( 0x1234567812345678, another.location.key );
	assertEquals( 0x1234, another.location.keybits );
	assertEquals( 0xFFFF123412345678, another.hash );
}

