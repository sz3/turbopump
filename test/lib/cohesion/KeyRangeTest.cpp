/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "KeyRange.h"
#include "data_structures/merkle_location.h"
#include "serialize/StringUtil.h"

// big endian!!!

TEST_CASE( "KeyRangeTest/testMaxBits", "[unit]" )
{
	{
		merkle_location<unsigned long long> loc(1455069594437219180ULL, 64);
		KeyRange range(loc);
		assertEquals( loc.key, range.first() );
		assertEquals( loc.key, range.last() );
	}
}

TEST_CASE( "KeyRangeTest/testDefault", "[unit]" )
{
	{
		merkle_location<unsigned long long> loc(~0ULL, 1);
		KeyRange range(loc);
		assertEquals( 128, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(~0ULL, 2);
		KeyRange range(loc);
		assertEquals( 192, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(~0ULL, 5);
		KeyRange range(loc);
		assertEquals( 248, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(~0ULL, 0);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 0);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 1);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF7FULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 2);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF3FULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 63);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0x100000000000000ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 62);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0x300000000000000ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 60);
		KeyRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xF00000000000000ULL, range.last() );
	}


	{
		merkle_location<unsigned long long> loc(128, 1);
		KeyRange range(loc);
		assertEquals( 128, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(2112, 2);
		KeyRange range(loc);
		assertEquals( 64, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF7FUL, range.last() );
	}


	{
		merkle_location<unsigned long long> loc(0x84315, 56);
		KeyRange range(loc);
		assertEquals( 0x84315, range.first() );
		assertEquals( 0xFF00000000084315ULL, range.last() );
	}
}
