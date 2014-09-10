/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

// explore interaction between changes in ConsistentHashRing and KeyTabulator, validating everything stays sane

#include "KeyTabulator.h"
#include "consistent_hashing/ConsistentHashRing.h"
#include "consistent_hashing/Hash.h"
#include "consistent_hashing/LocateKeys.h"
#include "deskew/IDigestKeys.h"
#include "deskew/DigestTree.h"

#include "membership/Peer.h"
#include "mock/MockMembership.h"
#include "serialize/StringUtil.h"
using std::string;
using std::vector;

// grow and shrink the hash ring and merkle index, verifying things stay sane
TEST_CASE( "KeyTabulatorDynamicHashRingTest/testShrinkGrow", "[integration]" )
{
	MockMembership membership;
	membership._self->uid = "1";

	// baseline: create 5 ranges
	ConsistentHashRing ring;
	for (unsigned i = 1; i <= 5; ++i)
	{
		string worker = StringUtil::str(i);
		ring.insert(worker, worker);
		std::cout << worker << " = " << Hash(worker).base64() << std::endl;
	}

	LocateKeys locator(ring, membership);
	KeyTabulator baseLine(locator);
	KeyTabulator index(locator);
	for (unsigned i = 50; i > 0; --i)
	{
		string file = StringUtil::str(i);
		baseLine.update(file, 0);
		index.update(file, 0);
	}

	index.print();
	std::cout << " *** 2 *** " << std::endl;
	((const DigestTree&)index.find(Hash("2").base64())).print(2);

	assertEquals( "26 46 6 40 9 21 41 32 10 29 8 1", StringUtil::join(index.find(Hash("1").base64()).enumerate(0, ~0ULL)) );
	assertEquals( "2 15 37 18 19 42 12 31 43 50 22", StringUtil::join(index.find(Hash("2").base64()).enumerate(0, ~0ULL)) );
	assertEquals( "17 14 20 48 34 36 44 11 24 16 23 38 30 39 45 35 3", StringUtil::join(index.find(Hash("3").base64()).enumerate(0, ~0ULL)) );
	assertEquals( "4", StringUtil::join(index.find(Hash("4").base64()).enumerate(0, ~0ULL)) );
	assertEquals( "28 33 49 7 13 25 47 27 5", StringUtil::join(index.find(Hash("5").base64()).enumerate(0, ~0ULL)) );

	// merge sections 4 and 5 (5 happens to hash to *rightbefore* 4, which is why 4 has only one key)
	index.cannibalizeSection("5");
	ring.erase("5");

	assertEquals( "", StringUtil::join(index.find(Hash("5").base64()).enumerate(0, ~0ULL)) );
	assertEquals( "28 33 49 7 13 25 47 27 5 4", StringUtil::join(index.find(Hash("4").base64()).enumerate(0, ~0ULL)) );

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
		string worker = StringUtil::str(i);
		assertEquals( StringUtil::join(baseLine.find(Hash(worker).base64()).enumerate(0, ~0ULL)),
					  StringUtil::join(index.find(Hash(worker).base64()).enumerate(0, ~0ULL)) );
	}
}
