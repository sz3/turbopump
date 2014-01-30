/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DigestIndexer.h"

#include "KeyRange.h"
#include "consistent_hashing/Hash.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "serialize/StringUtil.h"
#include <deque>
#include <string>
using std::deque;
using std::string;

TEST_CASE( "DigestIndexerTest/testNoRingMembers", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	DigestIndexer index(ring, membership);

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

TEST_CASE( "DigestIndexerTest/testSingleTree", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("fooid");
	MockMembership membership;
	DigestIndexer index(ring, membership);

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

TEST_CASE( "DigestIndexerTest/testManyTrees", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	MockMembership membership;
	DigestIndexer index(ring, membership);
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
	class TestableDigestIndexer : public DigestIndexer
	{
	public:
		TestableDigestIndexer(IHashRing& ring, IMembership& membership)
			: DigestIndexer(ring, membership)
		{}

	public:
		using DigestIndexer::_unwanted;
		using DigestIndexer::_wanted;
	};
}

TEST_CASE( "DigestIndexerTest/testWantedAndUnwanted", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

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

TEST_CASE( "DigestIndexerTest/testWantedAndUnwanted.NoRing", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

	index.add("one");
	index.add("two");
	assertEquals( "section(one)|locationsFromHash(,3)|section(two)", ring._history.calls() );
	assertEquals( 1, index.list().size() );
	assertEquals( 0, index._unwanted.size() );
	assertEquals( 1, index._wanted.size() );
}

TEST_CASE( "DigestIndexerTest/testSplitSection.InHalf", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");

	ring._workers[0] = "zzz";
	index.add("three");
	index.add("four");
	index.add("five");

	ring._workers[0] = "one";
	ring._history.clear();
	index.splitSection("one");

	assertEquals( "aaa one zzz", StringUtil::join(index.list()) );
	assertEquals( "aaa one zzz", StringUtil::join(index._unwanted) );
	assertEquals( "one", index.find("one").id().id );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", StringUtil::join(files) );

	files = index.find("one").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "five three", StringUtil::join(files) );

	files = index.find("zzz").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "four", StringUtil::join(files) );

	assertEquals( "section(one)|locationsFromHash(one,3)", ring._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testSplitSection.NoKeys", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

	ring._workers.push_back("two");
	index.add("one");
	index.add("two");
	index.add("three");

	ring._workers[0] = "13";
	ring._history.clear();
	index.splitSection("13");

	// but 13 doesn't have any keys, so he gets nothing.
	assertEquals( "two", StringUtil::join(index.list()) );
	assertEquals( "two", StringUtil::join(index._unwanted) );

	deque<string> files = index.find("two").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", StringUtil::join(files) );

	assertEquals( "section(13)|locationsFromHash(13,3)", ring._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testSplitSection.BecomeFirst", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

	ring._workers.push_back(Hash::compute("four").base64());
	index.add("one");
	index.add("two");
	index.add("three");

	ring._workers[0] = Hash::compute("one").base64();
	ring._history.clear();
	index.splitSection("one");

	// one takes all of four's keys!
	assertEquals( ring._workers[0], StringUtil::join(index.list()) );
	assertEquals( ring._workers[0], StringUtil::join(index._unwanted) );
	assertEquals( ring._workers[0], index.find(ring._workers[0]).id().id );

	deque<string> files = index.find(ring._workers[0]).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", StringUtil::join(files) );

	assertStringsEqual( "section(one)|locationsFromHash(" + ring._workers[0] + ",3)", ring._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testSplitSection.BecomeLast", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);
	ring._workers.push_back(Hash::compute("2").base64());
	index.add("one");
	index.add("two");
	index.add("three");

	//((const MerkleTree&)index.find(Hash::compute("2").base64())).print(5);

	ring._workers[0] = Hash::compute("four").base64();
	ring._history.clear();
	index.splitSection("four");

	deque<string> files = index.find(ring._workers[0]).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", StringUtil::join(files) );

	// four takes all of 2's keys!
	assertEquals( ring._workers[0], StringUtil::join(index.list()) );
	assertEquals( ring._workers[0], StringUtil::join(index._unwanted) );
	assertEquals( ring._workers[0], index.find(ring._workers[0]).id().id );

	assertStringsEqual( "section(four)|locationsFromHash(" + ring._workers[0] + ",3)", ring._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testSplitEmptyTree", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

	ring._workers.push_back("one");
	ring._history.clear();
	index.splitSection("one");
	assertEquals( "", StringUtil::join(index.list()) );

	assertEquals( "", ring._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testCannibalizeSection.Last", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

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

TEST_CASE( "DigestIndexerTest/testCannibalizeSection.First", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

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

TEST_CASE( "DigestIndexerTest/testCannibalizeSection.Middle", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

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
	assertEquals( "two one", StringUtil::join(files) );

	files = index.find("zzz").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "five three four", StringUtil::join(files) );
}

// try not to eat the last section
TEST_CASE( "DigestIndexerTest/testCannibalizeSection.ToEmpty", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	TestableDigestIndexer index(ring, membership);

	ring._workers.push_back("aaa");
	index.add("one");
	index.add("two");

	index.cannibalizeSection("aaa");
	assertEquals( "aaa", StringUtil::join(index.list()) );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", StringUtil::join(files) );
}