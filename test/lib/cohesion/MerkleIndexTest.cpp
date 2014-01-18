/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MerkleIndex.h"
#include "MerkleRange.h"
#include "consistent_hashing/Hash.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "serialize/StringUtil.h"
#include <deque>
#include <string>
using std::deque;
using std::string;

TEST_CASE( "MerkleIndexTest/testNoRingMembers", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MerkleIndex index(ring, membership);

	index.add("one");
	index.add("two");
	index.add("three");

	deque<string> files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two three one", StringUtil::join(files) );

	index.remove("two");
	index.remove("three");

	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one", StringUtil::join(files) );

	index.add("four");
	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one four", StringUtil::join(files) );

	index.remove("one");
	index.remove("four");
	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", StringUtil::join(files) );
}

TEST_CASE( "MerkleIndexTest/testSingleTree", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("fooid");
	MockMembership membership;
	MerkleIndex index(ring, membership);

	index.add("one");
	index.add("two");
	index.add("three");

	deque<string> files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two three one", StringUtil::join(files) );

	index.remove("two");
	index.remove("three");

	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one", StringUtil::join(files) );

	index.add("four");
	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one four", StringUtil::join(files) );

	index.remove("one");
	index.remove("four");
	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", StringUtil::join(files) );
}

TEST_CASE( "MerkleIndexTest/testManyTrees", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	MockMembership membership;
	MerkleIndex index(ring, membership);
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
	assertStringsEqual( "two one", StringUtil::join(files) );

	files = index.find("bbb").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "three four", StringUtil::join(files) );

	files = index.find("ccc").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "five six", StringUtil::join(files) );

	index.remove("five");
	index.remove("six");
	assertEquals( 2, index.list().size() );

	files = index.find("ccc").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", StringUtil::join(files) );
}

namespace {
	class TestableMerkleIndex : public MerkleIndex
	{
	public:
		TestableMerkleIndex(IHashRing& ring, IMembership& membership)
			: MerkleIndex(ring, membership)
		{}

	public:
		using MerkleIndex::_unwanted;
		using MerkleIndex::_wanted;
	};
}

TEST_CASE( "MerkleIndexTest/testWantedAndUnwanted", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	MockMembership membership;
	TestableMerkleIndex index(ring, membership);

	index.add("one");
	index.add("two");
	assertEquals( "aaa", StringUtil::join(index.list()) );
	assertEquals( "aaa", StringUtil::join(index._unwanted) );
	assertEquals( "", StringUtil::join(index._wanted) );

	ring._workers[0] = "me";
	index.add("three");
	index.add("four");
	assertEquals( "aaa me", StringUtil::join(index.list()) );
	assertEquals( "aaa", StringUtil::join(index._unwanted) );
	assertEquals( "me", StringUtil::join(index._wanted) );

	// randomTree picks from wanted trees -- ones we should be doing merkle exchanges for
	assertEquals( "me", index.randomTree().id() );
	assertEquals( "aaa", index.unwantedTree().id() );

	index.remove("three");
	index.remove("four");
	assertEquals( "aaa", StringUtil::join(index.list()) );
	assertEquals( "aaa", StringUtil::join(index._unwanted) );
	assertEquals( "", StringUtil::join(index._wanted) );

	ring._workers[0] = "aaa";
	index.remove("one");
	index.remove("two");
	assertEquals( "", StringUtil::join(index.list()) );
	assertEquals( "", StringUtil::join(index._unwanted) );
}
