/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MerkleIndex.h"
#include "MerkleRange.h"
#include "consistent_hashing/Hash.h"
#include "mock/MockHashRing.h"
#include "serialize/StringUtil.h"
#include <deque>
#include <string>
using std::deque;
using std::string;

TEST_CASE( "MerkleIndexTest/testNoRingMembers", "[unit]" )
{
	MockHashRing ring;
	MerkleIndex index(ring);

	index.add("one");
	index.add("two");
	index.add("three");

	deque<string> files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two three one", StringUtil::stlJoin(files) );

	index.remove("two");
	index.remove("three");

	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one", StringUtil::stlJoin(files) );

	index.add("four");
	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one four", StringUtil::stlJoin(files) );

	index.remove("one");
	index.remove("four");
	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", StringUtil::stlJoin(files) );
}

TEST_CASE( "MerkleIndexTest/testSingleTree", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("fooid");
	MerkleIndex index(ring);

	index.add("one");
	index.add("two");
	index.add("three");

	deque<string> files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two three one", StringUtil::stlJoin(files) );

	index.remove("two");
	index.remove("three");

	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one", StringUtil::stlJoin(files) );

	index.add("four");
	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one four", StringUtil::stlJoin(files) );

	index.remove("one");
	index.remove("four");
	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", StringUtil::stlJoin(files) );
}

TEST_CASE( "MerkleIndexTest/testManyTrees", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	MerkleIndex index(ring);
	assertEquals( 0, index.list().size() );

	index.add("one");
	index.add("two");
	assertEquals( 1, index.list().size() );

	ring._workers[0] = "bbb";
	index.add("three");
	index.add("four");
	assertEquals( 2, index.list().size() );

	ring._workers[0] = "ccc";
	index.add("five");
	index.add("six");
	assertEquals( 3, index.list().size() );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two one", StringUtil::stlJoin(files) );

	files = index.find("bbb").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "three four", StringUtil::stlJoin(files) );

	files = index.find("ccc").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "five six", StringUtil::stlJoin(files) );

	index.remove("five");
	index.remove("six");
	assertEquals( 2, index.list().size() );

	files = index.find("ccc").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", StringUtil::stlJoin(files) );
}
