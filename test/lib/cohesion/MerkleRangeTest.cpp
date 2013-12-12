#include "unittest.h"

#include "data_structures/merkle_location.h"
#include "MerkleRange.h"
#include "serialize/StringUtil.h"

// big endian!!!

TEST_CASE( "MerkleRangeTest/testMaxBits", "[unit]" )
{
	{
		merkle_location<unsigned long long> loc(1455069594437219180ULL, 64);
		MerkleRange range(loc);
		assertEquals( loc.key, range.first() );
		assertEquals( loc.key, range.last() );
	}
}

TEST_CASE( "MerkleRangeTest/testDefault", "[unit]" )
{
	{
		merkle_location<unsigned long long> loc(~0ULL, 1);
		MerkleRange range(loc);
		assertEquals( 128, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(~0ULL, 2);
		MerkleRange range(loc);
		assertEquals( 192, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(~0ULL, 5);
		MerkleRange range(loc);
		assertEquals( 248, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(~0ULL, 0);
		MerkleRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 0);
		MerkleRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 1);
		MerkleRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF7FULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 2);
		MerkleRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF3FULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 63);
		MerkleRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0x100000000000000ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 62);
		MerkleRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0x300000000000000ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(0, 60);
		MerkleRange range(loc);
		assertEquals( 0, range.first() );
		assertEquals( 0xF00000000000000ULL, range.last() );
	}


	{
		merkle_location<unsigned long long> loc(128, 1);
		MerkleRange range(loc);
		assertEquals( 128, range.first() );
		assertEquals( ~0ULL, range.last() );
	}

	{
		merkle_location<unsigned long long> loc(2112, 2);
		MerkleRange range(loc);
		assertEquals( 64, range.first() );
		assertEquals( 0xFFFFFFFFFFFFFF7FUL, range.last() );
	}


	{
		merkle_location<unsigned long long> loc(0x84315, 56);
		MerkleRange range(loc);
		assertEquals( 0x84315, range.first() );
		assertEquals( 0xFF00000000084315ULL, range.last() );
	}
}
