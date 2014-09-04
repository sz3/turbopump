/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

// explore interaction between changes in HashRing and KeyTabulator, validating everything stays sane

#include "KeyTabulator.h"
#include "consistent_hashing/HashRing.h"
#include "deskew/IDigestKeys.h"

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
	HashRing ring;
	for (unsigned i = 1; i <= 5; ++i)
	{
		string worker = StringUtil::str(i);
		ring.addWorker(worker);
		std::cout << worker << " = " << HashRing::hash(worker) << std::endl;
	}

	KeyTabulator baseLine(ring, membership);
	KeyTabulator index(ring, membership);
	for (unsigned i = 50; i > 0; --i)
	{
		string file = StringUtil::str(i);
		baseLine.update(file, 0);
		index.update(file, 0);
	}

	//index.print();
	//std::cout << " *** 2 *** " << std::endl;
	//((const MerkleTree&)index.find(HashRing::hash("2"))).print(2);

	assertEquals( "26 46 6 40 9 21 41 32 10 29 8 1", StringUtil::join(index.find(HashRing::hash("1")).enumerate(0, ~0ULL)) );
	assertEquals( "2 15 37 18 19 42 12 31 43 50 22", StringUtil::join(index.find(HashRing::hash("2")).enumerate(0, ~0ULL)) );
	assertEquals( "17 14 20 48 34 36 44 11 24 16 23 38 30 39 45 35 3", StringUtil::join(index.find(HashRing::hash("3")).enumerate(0, ~0ULL)) );
	assertEquals( "4", StringUtil::join(index.find(HashRing::hash("4")).enumerate(0, ~0ULL)) );
	assertEquals( "28 33 49 7 13 25 47 27 5", StringUtil::join(index.find(HashRing::hash("5")).enumerate(0, ~0ULL)) );

	// merge sections 4 and 5 (5 happens to hash to *rightbefore* 4, which is why 4 has only one key)
	index.cannibalizeSection("5");
	ring.removeWorker("5");

	assertEquals( "", StringUtil::join(index.find(HashRing::hash("5")).enumerate(0, ~0ULL)) );
	assertEquals( "28 33 49 7 13 25 47 27 5 4", StringUtil::join(index.find(HashRing::hash("4")).enumerate(0, ~0ULL)) );

	index.cannibalizeSection("1");
	ring.removeWorker("1");
	index.cannibalizeSection("3");
	ring.removeWorker("3");

	ring.addWorker("1");
	index.splitSection("1");
	ring.addWorker("5");
	index.splitSection("5");
	ring.addWorker("3");
	index.splitSection("3");

	// verify that everything ends up as it started
	for (unsigned i = 1; i <= 5; ++i)
	{
		string worker = StringUtil::str(i);
		assertEquals( StringUtil::join(baseLine.find(HashRing::hash(worker)).enumerate(0, ~0ULL)), StringUtil::join(index.find(HashRing::hash(worker)).enumerate(0, ~0ULL)) );
	}
}
