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
		assertInRange( 2, id.mirrors, 3 );
	}

	for (unsigned i = 0; i < 10; ++i)
	{
		TreeId id = index.unwantedTree().id();
		assertEquals( "fooid", id.id );
		assertInRange( 1, id.mirrors, 3 );
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
	// TODO: this test excercises the logic in a contrived way.
	// it would be nice to have a clearer example.
	MockLocateKeys locator;
	KeyTabulator index(locator);

	// section IDs are Hashes, and they should >= 64 bits.
	// in the wild, they'll likely be something like a sha256
	locator._locations.push_back("20000000");
	for (int i = 1; i <= 9; ++i)
	{
		string name = turbo::str::str(i);
		index.update(name + "0000000", 0, 1);
		index.update(name+name+name+name + "0000", 0, 2);
		index.update(name+name+name+name+name+name+name+name, 0, 3);
	}

	/*
	((const MerkleTree&)index.find("1",1)).print();
	((const MerkleTree&)index.find("1",2)).print();
	((const MerkleTree&)index.find("1",3)).print();
	//*/

	locator._locations[0] = "33333333";
	index.splitSection("33333333");

	deque<string> files = index.find("20000000", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "77777777 88888888 22222222 11111111 44444444 99999999", turbo::str::join(files) );

	files = index.find("33333333", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "55555555 66666666 33333333", turbo::str::join(files) );

	files = index.find("20000000", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "22220000 99990000 66660000 33330000 55550000 44440000 88880000", turbo::str::join(files) );

	files = index.find("33333333", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "11110000 77770000", turbo::str::join(files) );

	files = index.find("20000000", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "30000000 70000000 50000000 60000000 80000000 10000000 20000000", turbo::str::join(files) );

	files = index.find("33333333", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "40000000 90000000", turbo::str::join(files) );

	// split again
	locator._locations[0] = "44440000";
	index.splitSection("44440000");

	files = index.find("33333333", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "55555555 66666666 33333333", turbo::str::join(files) );

	files = index.find("44440000", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "22222222 11111111 44444444", turbo::str::join(files) );

	files = index.find("20000000", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "77777777 88888888 99999999", turbo::str::join(files) );

	files = index.find("33333333", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "11110000 77770000", turbo::str::join(files) );

	files = index.find("44440000", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "66660000 33330000 55550000 44440000", turbo::str::join(files) );

	files = index.find("20000000", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "22220000 99990000 88880000", turbo::str::join(files) );

	files = index.find("33333333", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "40000000 90000000", turbo::str::join(files) );

	files = index.find("44440000", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "70000000 50000000 60000000 80000000 10000000 20000000", turbo::str::join(files) );

	files = index.find("20000000", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "30000000", turbo::str::join(files) );


	// remove section -- go back to how things were
	locator._locations[0] = "44440000";
	index.cannibalizeSection("44440000");

	files = index.find("20000000", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "77777777 88888888 22222222 11111111 44444444 99999999", turbo::str::join(files) );

	files = index.find("33333333", 3).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "55555555 66666666 33333333", turbo::str::join(files) );

	files = index.find("20000000", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "22220000 99990000 66660000 33330000 55550000 44440000 88880000", turbo::str::join(files) );

	files = index.find("33333333", 2).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "11110000 77770000", turbo::str::join(files) );

	files = index.find("20000000", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "30000000 70000000 50000000 60000000 80000000 10000000 20000000", turbo::str::join(files) );

	files = index.find("33333333", 1).enumerate(0, 0xFFFFFFFFFFFFFFFFULL);
	assertEquals( "40000000 90000000", turbo::str::join(files) );
}
