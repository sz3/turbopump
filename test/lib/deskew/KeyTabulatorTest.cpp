/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "KeyTabulator.h"

#include "IDigestKeys.h"
#include "KeyRange.h"
#include "TreeId.h"
#include "mock/MockLocateKeys.h"
#include "serialize/str.h"
#include "serialize/str_join.h"
#include <deque>
#include <string>
using std::deque;
using std::string;

TEST_CASE( "KeyTabulatorTest/testNoRingMembers", "[unit]" )
{
	MockLocateKeys locator;
	KeyTabulator index(locator);

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

TEST_CASE( "KeyTabulatorTest/testSingleTree", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("fooid");
	KeyTabulator index(locator);

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

TEST_CASE( "KeyTabulatorTest/testRandomAndUnwanted", "[unit]" )
{
	MockLocateKeys locator;
	KeyTabulator index(locator);

	locator._locations.push_back("fooid");
	locator._mine = false;
	index.update("unwanted1", 0, 1);
	index.update("unwanted2", 0, 2);
	index.update("unwanted3", 0, 3);

	locator._locations[0] = "me";
	locator._mine = true;
	index.update("wanted1", 0, 1);
	index.update("wanted2", 0, 2);
	index.update("wanted3", 0, 3);

	for (unsigned i = 0; i < 10; ++i)
	{
		TreeId id = index.randomTree().id();
		assertEquals( "me", id.id );
		assertInRange( 2, 3, id.mirrors );
	}

	for (unsigned i = 0; i < 10; ++i)
	{
		TreeId id = index.unwantedTree().id();
		assertEquals( "fooid", id.id );
		assertInRange( 1, 3, id.mirrors );
	}
}

TEST_CASE( "KeyTabulatorTest/testRandomAndUnwanted.Exclude", "[unit]" )
{
	MockLocateKeys locator;
	KeyTabulator index(locator);

	locator._locations.push_back("fooid");
	locator._mine = false;
	index.update("unwanted1", 0, 1);

	locator._locations[0] = "me";
	locator._mine = true;
	index.update("wanted1", 0, 1);

	{
		const IDigestKeys& tree = index.randomTree();
		assertEquals( "", tree.id().id );
		assertTrue( tree.empty() );
	}

	{
		TreeId id = index.unwantedTree().id();
		assertEquals( "fooid", id.id );
		assertEquals( 1, id.mirrors );
	}
}

TEST_CASE( "KeyTabulatorTest/testRandomAndUnwanted.0", "[unit]" )
{
	MockLocateKeys locator;
	KeyTabulator index(locator);

	locator._locations.push_back("fooid");
	locator._mine = false;
	index.update("wanted1", 0, 0);

	{
		const IDigestKeys& tree = index.randomTree();
		assertFalse( tree.empty() );
		assertEquals( "", tree.id().id );
		assertEquals( 0, tree.id().mirrors );
	}


	{
		const IDigestKeys& tree = index.unwantedTree();
		assertTrue( tree.empty() );
		assertEquals( "", tree.id().id );
	}
}

TEST_CASE( "KeyTabulatorTest/testRandomAndUnwanted.0_1", "[unit]" )
{
	MockLocateKeys locator;
	KeyTabulator index(locator);

	locator._locations.push_back("fooid");
	locator._mine = false;
	index.update("wanted1", 0, 0);
	index.update("unwanted1", 0, 1);

	{
		const IDigestKeys& tree = index.randomTree();
		assertFalse( tree.empty() );
		assertEquals( "", tree.id().id );
		assertEquals( 0, tree.id().mirrors );
	}


	{
		const IDigestKeys& tree = index.unwantedTree();
		assertFalse( tree.empty() );
		assertEquals( "fooid", tree.id().id );
		assertEquals( 1, tree.id().mirrors );
	}
}

TEST_CASE( "KeyTabulatorTest/testUnwanted.0_3", "[unit]" )
{
	MockLocateKeys locator;
	KeyTabulator index(locator);

	locator._locations.push_back("fooid");
	locator._mine = false;
	index.update("wanted1", 0, 0);
	index.update("unwanted1", 0, 3);

	{
		const IDigestKeys& tree = index.unwantedTree();
		assertFalse( tree.empty() );
		assertEquals( "fooid", tree.id().id );
		assertEquals( 3, tree.id().mirrors );
	}
}

TEST_CASE( "KeyTabulatorTest/testReorganizeSections", "[unit]" )
{
	MockLocateKeys locator;
	KeyTabulator index(locator);

	locator._locations.push_back("2");
	for (int i = 1; i <= 9; ++i)
	{
		string name = turbo::str::str(i);
		index.update(name, 0, 1);
		index.update(name+name, 0, 2);
		index.update(name+name+name, 0, 3);
	}

	/*
	((const MerkleTree&)index.find("1",1)).print();
	((const MerkleTree&)index.find("1",2)).print();
	((const MerkleTree&)index.find("1",3)).print();
	//*/

	locator._locations[0] = "555";
	index.splitSection("555");

	deque<string> files = index.find("2", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "777 111", turbo::str::join(files) );

	files = index.find("555", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "888 999 222 666 333 444 555", turbo::str::join(files) );

	files = index.find("2", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "77 22", turbo::str::join(files) );

	files = index.find("555", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "55 33 99 88 66 44 11", turbo::str::join(files) );

	files = index.find("2", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "3", turbo::str::join(files) );

	files = index.find("555", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "2 6 9 8 1 7 5 4", turbo::str::join(files) );


	locator._locations[0] = "44";
	index.splitSection("44");

	files = index.find("555", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "333 444 555", turbo::str::join(files) );

	files = index.find("44", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "888 999 222 666", turbo::str::join(files) );

	files = index.find("555", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "11", turbo::str::join(files) );

	files = index.find("44", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "55 33 99 88 66 44", turbo::str::join(files) );

	files = index.find("555", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "8 1 7 5 4", turbo::str::join(files) );

	files = index.find("44", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "2 6 9", turbo::str::join(files) );


	/*locator._locations[0] = "2";
	index.cannibalizeSection("2");

	files = index.find("555", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "888 999 222 666", turbo::str::join(files) );

	files = index.find("44", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "333 444", turbo::str::join(files) );

	files = index.find("555", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "55 33 99 88 66", turbo::str::join(files) );

	files = index.find("44", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "44 11 77", turbo::str::join(files) );

	files = index.find("555", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "2 6 9 8 1 7 5 4", turbo::str::join(files) );

	files = index.find("44", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "", turbo::str::join(files) );*/
}
