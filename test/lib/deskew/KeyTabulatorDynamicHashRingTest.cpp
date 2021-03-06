/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

// explore interaction between changes in ConsistentHashRing and KeyTabulator, validating everything stays sane

#include "KeyTabulator.h"
#include "deskew/IDigestKeys.h"
#include "deskew/DigestTree.h"
#include "hashing/ConsistentHashRing.h"
#include "hashing/Hash.h"
#include "hashing/LocateKeys.h"

#include "membership/Peer.h"
#include "mock/MockKnownPeers.h"
#include "serialize/str.h"
#include "serialize/str_join.h"
using std::string;
using std::vector;
using turbo::str::str;

// grow and shrink the hash ring and merkle index, verifying things stay sane
TEST_CASE( "KeyTabulatorDynamicHashRingTest/testShrinkGrow", "[integration]" )
{
	MockKnownPeers membership;
	membership._self->uid = "1";

	// baseline: create 5 ranges
	ConsistentHashRing ring;
	for (unsigned i = 1; i <= 5; ++i)
	{
		string worker = str(i);
		ring.insert(worker, worker);
		std::cout << worker << " = " << Hash(worker).base64() << std::endl;
	}

	LocateKeys locator(ring, membership);
	KeyTabulator baseLine(locator);
	KeyTabulator index(locator);
	for (unsigned i = 50; i > 0; --i)
	{
		string file = str(i);
		baseLine.update(file, 0);
		index.update(file, 0);
	}

	index.print();
	std::cout << " *** 2 *** " << std::endl;
	((const DigestTree&)index.find(Hash("2").str())).print(2);

	assertEquals( "26 46 6 40 9 21 41 32 10 29 8 1", turbo::str::join(index.find(Hash("1").str()).enumerate(0, ~0ULL)) );
	assertEquals( "2 15 37 18 19 42 12 31 43 50 22", turbo::str::join(index.find(Hash("2").str()).enumerate(0, ~0ULL)) );
	assertEquals( "17 14 20 48 34 36 44 11 24 16 23 38 30 39 45 35 3", turbo::str::join(index.find(Hash("3").str()).enumerate(0, ~0ULL)) );
	assertEquals( "4", turbo::str::join(index.find(Hash("4").str()).enumerate(0, ~0ULL)) );
	assertEquals( "28 33 49 7 13 25 47 27 5", turbo::str::join(index.find(Hash("5").str()).enumerate(0, ~0ULL)) );

	// merge sections 4 and 5 (5 happens to hash to *rightbefore* 4, which is why 4 has only one key)
	index.cannibalizeSection("5");
	ring.erase("5");

	assertEquals( "", turbo::str::join(index.find(Hash("5").str()).enumerate(0, ~0ULL)) );
	assertEquals( "28 33 49 7 13 25 47 27 5 4", turbo::str::join(index.find(Hash("4").str()).enumerate(0, ~0ULL)) );

	index.cannibalizeSection("1");
	ring.erase("1");
	index.cannibalizeSection("3");
	ring.erase("3");

	ring.insert("1", "1");
	index.splitSection("1");
	ring.insert("5", "5");
	index.splitSection("5");
	ring.insert("3", "3");
	index.splitSection("3");

	// verify that everything ends up as it started
	for (unsigned i = 1; i <= 5; ++i)
	{
		string worker = str(i);
		assertEquals( turbo::str::join(baseLine.find(Hash(worker).str()).enumerate(0, ~0ULL)),
					  turbo::str::join(index.find(Hash(worker).str()).enumerate(0, ~0ULL)) );
	}
}
