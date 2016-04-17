/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "KeyRange.h"
#include "common/MerklePoint.h"

// big endian!!!

TEST_CASE( "KeyRangeTest/testMaxBits", "[unit]" )
{
	{
		MerklePoint loc(1234, 1455069594437219180ULL, 64);
		KeyRange range(loc);
		assertEquals( loc.key, range.first() );
		assertEquals( loc.key, range.last() );
	}
}

TEST_CASE( "KeyRangeTest/testDefault", "[unit]" )
{
	{
		MerklePoint loc(1234, ~0ULL, 1);
		KeyRange range(loc);
		assertEquals( 128, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		MerklePoint loc(1234, ~0ULL, 2);
		KeyRange range(loc);
		assertEquals( 192, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		MerklePoint loc(1234, ~0ULL, 5);
		KeyRange range(loc);
		assertEquals( 248, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		MerklePoint loc(1234, ~0ULL, 0);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		MerklePoint loc(1234, 0, 0);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		MerklePoint loc(1234, 0, 1);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF7FULL, range.last() );
	}

	{
		MerklePoint loc(1234, 0, 2);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF3FULL, range.last() );
	}

	{
		MerklePoint loc(1234, 0, 63);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0x100000000000000ULL, range.last() );
	}

	{
		MerklePoint loc(1234, 0, 62);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0x300000000000000ULL, range.last() );
	}

	{
		MerklePoint loc(1234, 0, 60);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xF00000000000000ULL, range.last() );
	}


	{
		MerklePoint loc(1234, 128, 1);
		KeyRange range(loc);
		assertEquals( 128, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		MerklePoint loc(1234, 2112, 2);
		KeyRange range(loc);
		assertEquals( 64, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF7FUL, range.last() );
	}


	{
		MerklePoint loc(1234, 0x84315, 56);
		KeyRange range(loc);
		assertEquals( 0x84315, range.first() );
		assertEquals( 0xFF00000000084315ULL, range.last() );
	}
}
