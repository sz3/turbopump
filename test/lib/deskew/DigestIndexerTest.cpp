/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "DigestIndexer.h"

#include "KeyRange.h"
#include "hashing/Hash.h"
#include "mock/MockLocateKeys.h"
#include "serialize/str_join.h"
#include <deque>
#include <string>
using std::deque;
using std::string;

TEST_CASE( "DigestIndexerTest/testNoRingMembers", "[unit]" )
{
	MockLocateKeys locator;
	DigestIndexer index(locator);

	index.update("one", 0);
	index.update("two", 0);
	index.update("three", 0);

	deque<string> files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two three one", turbo::str::join(files) );

	index.remove("two");
	index.remove("three");

	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one", turbo::str::join(files) );

	index.update("four", 0);
	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one four", turbo::str::join(files) );

	index.remove("one");
	index.remove("four");
	files = index.find("").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", turbo::str::join(files) );
}

TEST_CASE( "DigestIndexerTest/testSingleTree", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("fooid");
	DigestIndexer index(locator);

	index.update("one", 0);
	index.update("two", 0);
	index.update("three", 0);

	deque<string> files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two three one", turbo::str::join(files) );

	index.remove("two");
	index.remove("three");

	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one", turbo::str::join(files) );

	index.update("four", 0);
	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "one four", turbo::str::join(files) );

	index.remove("one");
	index.remove("four");
	files = index.find("fooid").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", turbo::str::join(files) );
}

TEST_CASE( "DigestIndexerTest/testManyTrees", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	DigestIndexer index(locator);
	assertEquals( 0, index.list().size() );

	index.update("one", 0);
	index.update("two", 0);
	assertEquals( 1, index.list().size() );

	locator._locations[0] = "bbb";
	index.update("three", 0);
	index.update("four", 0);
	assertEquals( 2, index.list().size() );

	locator._locations[0] = "ccc";
	index.update("five", 0);
	index.update("six", 0);
	assertEquals( 3, index.list().size() );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "two one", turbo::str::join(files) );

	files = index.find("bbb").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "three four", turbo::str::join(files) );

	files = index.find("ccc").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "five six", turbo::str::join(files) );

	index.remove("five");
	index.remove("six");
	assertEquals( 2, index.list().size() );

	files = index.find("ccc").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertStringsEqual( "", turbo::str::join(files) );
}

namespace {
	class TestableDigestIndexer : public DigestIndexer
	{
	public:
		TestableDigestIndexer(ILocateKeys& locator)
			: DigestIndexer(locator)
		{}

	public:
		using DigestIndexer::_unwanted;
		using DigestIndexer::_wanted;
	};
}

TEST_CASE( "DigestIndexerTest/testWantedAndUnwanted", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._mine = false;
	TestableDigestIndexer index(locator);

	index.update("one", 0);
	index.update("two", 0);
	assertEquals( "section(one)|sectionIsMine(aaa,3)|section(two)", locator._history.calls() );
	assertEquals( "aaa", turbo::str::join(index.list()) );
	assertEquals( "aaa", turbo::str::join(index._unwanted) );
	assertEquals( "", turbo::str::join(index._wanted) );

	locator._locations[0] = "me";
	locator._mine = true;
	locator._history.clear();
	index.update("three", 0);
	index.update("four", 0);
	assertEquals( "section(three)|sectionIsMine(me,3)|section(four)", locator._history.calls() );
	assertEquals( "aaa me", turbo::str::join(index.list()) );
	assertEquals( "aaa", turbo::str::join(index._unwanted) );
	assertEquals( "me", turbo::str::join(index._wanted) );

	// randomTree picks from wanted trees -- ones we should be doing merkle exchanges for
	assertEquals( "me", index.randomTree().id().id );
	assertEquals( "aaa", index.unwantedTree().id().id );

	index.remove("three");
	index.remove("four");
	assertEquals( "aaa", turbo::str::join(index.list()) );
	assertEquals( "aaa", turbo::str::join(index._unwanted) );
	assertEquals( "", turbo::str::join(index._wanted) );

	locator._locations[0] = "aaa";
	index.remove("one");
	index.remove("two");
	assertEquals( "", turbo::str::join(index.list()) );
	assertEquals( "", turbo::str::join(index._unwanted) );
}

TEST_CASE( "DigestIndexerTest/testWantedAndUnwanted.NoRing", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	index.update("one", 0);
	index.update("two", 0);
	assertEquals( "section(one)|sectionIsMine(,3)|section(two)", locator._history.calls() );
	assertEquals( 1, index.list().size() );
	assertEquals( 0, index._unwanted.size() );
	assertEquals( 1, index._wanted.size() );
}

TEST_CASE( "DigestIndexerTest/testSplitSection.InHalf", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	locator._mine = false;
	locator._locations.push_back("aaa");
	index.update("one", 0);
	index.update("two", 0);

	locator._locations[0] = "zzz";
	index.update("three", 0);
	index.update("four", 0);
	index.update("five", 0);

	locator._locations[0] = "one";
	locator._history.clear();
	index.splitSection("one");

	assertEquals( "aaa one zzz", turbo::str::join(index.list()) );
	assertEquals( "aaa one zzz", turbo::str::join(index._unwanted) );
	assertEquals( "one", index.find("one").id().id );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", turbo::str::join(files) );

	files = index.find("one").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "five three", turbo::str::join(files) );

	files = index.find("zzz").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "four", turbo::str::join(files) );

	assertEquals( "section(one)|sectionIsMine(one,3)", locator._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testSplitSection.NoKeys", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	locator._mine = false;
	locator._locations.push_back("two");
	index.update("one", 0);
	index.update("two", 0);
	index.update("three", 0);

	locator._locations[0] = "13";
	locator._history.clear();
	index.splitSection("13");

	// but 13 doesn't have any keys, so he gets nothing.
	assertEquals( "two", turbo::str::join(index.list()) );
	assertEquals( "two", turbo::str::join(index._unwanted) );

	deque<string> files = index.find("two").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", turbo::str::join(files) );

	assertEquals( "section(13)|sectionIsMine(13,3)", locator._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testSplitSection.BecomeFirst", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	locator._mine = false;
	locator._locations.push_back(Hash("four").base64());
	index.update("one", 0);
	index.update("two", 0);
	index.update("three", 0);

	locator._locations[0] = Hash("one").base64();
	locator._history.clear();
	index.splitSection("one");

	// one takes all of four's keys!
	assertEquals( locator._locations[0], turbo::str::join(index.list()) );
	assertEquals( locator._locations[0], turbo::str::join(index._unwanted) );
	assertEquals( locator._locations[0], index.find(locator._locations[0]).id().id );

	deque<string> files = index.find(locator._locations[0]).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", turbo::str::join(files) );

	assertStringsEqual( "section(one)|sectionIsMine(" + locator._locations[0] + ",3)", locator._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testSplitSection.BecomeLast", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);
	locator._mine = false;
	locator._locations.push_back(Hash("2").base64());
	index.update("one", 0);
	index.update("two", 0);
	index.update("three", 0);

	//((const MerkleTree&)index.find(Hash("2").base64())).print(5);

	locator._locations[0] = Hash("four").base64();
	locator._history.clear();
	index.splitSection("four");

	deque<string> files = index.find(locator._locations[0]).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one", turbo::str::join(files) );

	// four takes all of 2's keys!
	assertEquals( locator._locations[0], turbo::str::join(index.list()) );
	assertEquals( locator._locations[0], turbo::str::join(index._unwanted) );
	assertEquals( locator._locations[0], index.find(locator._locations[0]).id().id );

	assertStringsEqual( "section(four)|sectionIsMine(" + locator._locations[0] + ",3)", locator._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testSplitEmptyTree", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	locator._locations.push_back("one");
	locator._history.clear();
	index.splitSection("one");
	assertEquals( "", turbo::str::join(index.list()) );

	assertEquals( "", locator._history.calls() );
}

TEST_CASE( "DigestIndexerTest/testCannibalizeSection.Last", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	locator._mine = false;
	locator._locations.push_back("aaa");
	index.update("one", 0);
	index.update("two", 0);

	locator._locations[0] = "three";
	index.update("three", 0);
	index.update("four", 0);

	assertEquals( "aaa three", turbo::str::join(index.list()) );
	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", turbo::str::join(files) );

	index.cannibalizeSection("three");
	assertEquals( "aaa", turbo::str::join(index.list()) );
	assertEquals( "aaa", turbo::str::join(index._unwanted) );

	files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one four", turbo::str::join(files) );
}

TEST_CASE( "DigestIndexerTest/testCannibalizeSection.First", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	locator._mine = false;
	locator._locations.push_back("aaa");
	index.update("one", 0);
	index.update("two", 0);

	locator._locations[0] = "ccc";
	index.update("three", 0);
	index.update("four", 0);

	assertEquals( "aaa ccc", turbo::str::join(index.list()) );
	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", turbo::str::join(files) );

	locator._locations[0] = "aaa";
	index.cannibalizeSection("aaa");
	assertEquals( "ccc", turbo::str::join(index.list()) );
	assertEquals( "ccc", turbo::str::join(index._unwanted) );

	files = index.find("ccc").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two three one four", turbo::str::join(files) );
}

TEST_CASE( "DigestIndexerTest/testCannibalizeSection.Middle", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	locator._mine = false;
	locator._locations.push_back("aaa");
	index.update("one", 0);
	index.update("two", 0);

	locator._locations[0] = "ccc";
	index.update("three", 0);
	index.update("four", 0);

	locator._locations[0] = "zzz";
	index.update("five", 0);

	assertEquals( "aaa ccc zzz", turbo::str::join(index.list()) );

	locator._locations[0] = "ccc";
	index.cannibalizeSection("ccc");
	assertEquals( "aaa zzz", turbo::str::join(index.list()) );
	assertEquals( "aaa zzz", turbo::str::join(index._unwanted) );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", turbo::str::join(files) );

	files = index.find("zzz").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "five three four", turbo::str::join(files) );
}

// try not to eat the last section
TEST_CASE( "DigestIndexerTest/testCannibalizeSection.ToEmpty", "[unit]" )
{
	MockLocateKeys locator;
	TestableDigestIndexer index(locator);

	locator._locations.push_back("aaa");
	locator._mine = false;
	index.update("one", 0);
	index.update("two", 0);

	index.cannibalizeSection("aaa");
	assertEquals( "aaa", turbo::str::join(index.list()) );

	deque<string> files = index.find("aaa").enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "two one", turbo::str::join(files) );
}
