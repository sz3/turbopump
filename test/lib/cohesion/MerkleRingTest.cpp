/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MerkleRing.h"

#include "KeyRange.h"
#include "consistent_hashing/Hash.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "serialize/StringUtil.h"
#include <deque>
#include <string>
using std::deque;
using std::string;

TEST_CASE( "MerkleRingTest/testNoRingMembers", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MerkleRing index(ring, membership);

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

TEST_CASE( "MerkleRingTest/testSingleTree", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("fooid");
	MockMembership membership;
	MerkleRing index(ring, membership);

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

TEST_CASE( "MerkleRingTest/testManyTrees", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	MockMembership membership;
	MerkleRing index(ring, membership);
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
	class TestableMerkleRing : public MerkleRing
	{
	public:
		TestableMerkleRing(IHashRing& ring, IMembership& membership)
			: MerkleRing(ring, membership)
		{}

	public:
		using MerkleRing::_unwanted;
		using MerkleRing::_wanted;
	};
}

TEST_CASE( "MerkleRingTest/testWantedAndUnwanted", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	index.add("one");
	index.add("two");
	assertEquals( "section(one)|locationsFromHash(aaa,3)|section(two)", ring._history.calls() );
	assertEquals( "aaa", StringUtil::join(index.list()) );
	assertEquals( "aaa", StringUtil::join(index._unwanted) );
	assertEquals( "", StringUtil::join(index._wanted) );

	ring._workers[0] = "me";
	ring._history.clear();
	index.add("three");
	index.add("four");
	assertEquals( "section(three)|locationsFromHash(me,3)|section(four)", ring._history.calls() );
	assertEquals( "aaa me", StringUtil::join(index.list()) );
	assertEquals( "aaa", StringUtil::join(index._unwanted) );
	assertEquals( "me", StringUtil::join(index._wanted) );

	// randomTree picks from wanted trees -- ones we should be doing merkle exchanges for
	assertEquals( "me", index.randomTree().id().id );
	assertEquals( "aaa", index.unwantedTree().id().id );

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

TEST_CASE( "MerkleRingTest/testSplitSection.InHalf", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");
	index.add("three");
	index.add("four");

	ring._workers[0] = "zzz";
	index.add("five");

	ring._workers[0] = "one";
	ring._history.clear();
	index.splitSection("one");
	assertEquals( "aaa one zzz", StringUtil::join(index.list()) );
	assertEquals( "aaa one zzz", StringUtil::join(index._unwanted) );
	assertEquals( "one", index.find("one").id().id );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three", StringUtil::join(files) );

	files = index.find("one").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "one four", StringUtil::join(files) );

	files = index.find("zzz").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "five", StringUtil::join(files) );

	assertEquals( "section(one)|locationsFromHash(one,3)", ring._history.calls() );
}

TEST_CASE( "MerkleRingTest/testSplitSection.AllKeys", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");
	index.add("three");
	index.add("four");

	ring._workers[0] = "two";
	ring._history.clear();
	index.splitSection("two");
	assertEquals( "two", StringUtil::join(index.list()) );
	assertEquals( "two", StringUtil::join(index._unwanted) );
	assertEquals( "two", index.find("two").id().id );

	deque<string> files = index.find("two").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one four", StringUtil::join(files) );

	assertEquals( "section(two)|locationsFromHash(two,3)", ring._history.calls() );
}

TEST_CASE( "MerkleRingTest/testSplitSection.NoKeys", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");
	index.add("three");

	ring._workers[0] = "four";
	ring._history.clear();
	index.splitSection("four");
	assertEquals( "aaa", StringUtil::join(index.list()) );
	assertEquals( "aaa", StringUtil::join(index._unwanted) );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", StringUtil::join(files) );

	assertEquals( "section(four)|locationsFromHash(four,3)", ring._history.calls() );
}

TEST_CASE( "MerkleRingTest/testSplitEmptyTree", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	ring._workers.push_back("one");
	ring._history.clear();
	index.splitSection("one");
	assertEquals( "", StringUtil::join(index.list()) );

	assertEquals( "", ring._history.calls() );
}

TEST_CASE( "MerkleRingTest/testCannibalizeSection.Last", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");

	ring._workers[0] = "three";
	index.add("three");
	index.add("four");

	assertEquals( "aaa three", StringUtil::join(index.list()) );
	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", StringUtil::join(files) );

	index.cannibalizeSection("three");
	assertEquals( "aaa", StringUtil::join(index.list()) );
	assertEquals( "aaa", StringUtil::join(index._unwanted) );

	files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one four", StringUtil::join(files) );
}

TEST_CASE( "MerkleRingTest/testCannibalizeSection.First", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");

	ring._workers[0] = "ccc";
	index.add("three");
	index.add("four");

	assertEquals( "aaa ccc", StringUtil::join(index.list()) );
	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", StringUtil::join(files) );

	ring._workers[0] = "aaa";
	index.cannibalizeSection("aaa");
	assertEquals( "ccc", StringUtil::join(index.list()) );
	assertEquals( "ccc", StringUtil::join(index._unwanted) );

	files = index.find("ccc").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one four", StringUtil::join(files) );
}

TEST_CASE( "MerkleRingTest/testCannibalizeSection.Middle", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");

	ring._workers[0] = "ccc";
	index.add("three");
	index.add("four");

	ring._workers[0] = "zzz";
	index.add("five");

	assertEquals( "aaa ccc zzz", StringUtil::join(index.list()) );

	ring._workers[0] = "ccc";
	index.cannibalizeSection("ccc");
	assertEquals( "aaa zzz", StringUtil::join(index.list()) );
	assertEquals( "aaa zzz", StringUtil::join(index._unwanted) );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one four", StringUtil::join(files) );

	files = index.find("zzz").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "five", StringUtil::join(files) );
}

// not sure about the contents of this test case, as far as desired behavior goes.
// however, at least it doesn't core.
TEST_CASE( "MerkleRingTest/testCannibalizeSection.ToEmpty", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableMerkleRing index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");

	index.cannibalizeSection("aaa");
	assertEquals( "", StringUtil::join(index.list()) );
}
