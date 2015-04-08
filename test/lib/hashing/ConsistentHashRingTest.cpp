/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ConsistentHashRing.h"
#include "Hash.h"
#include "serialize/str_join.h"
using std::string;
using std::vector;

namespace {
	class TestableConsistentHashRing : public ConsistentHashRing
	{
	public:
		using ConsistentHashRing::_ring;
	};
}

TEST_CASE( "ConsistentHashRingTest/testInsert", "[unit]" )
{
	TestableConsistentHashRing ring;
	assertTrue( ring.insert("one", "1") );
	assertEquals( "1", ring._ring[Hash("one").str()] );

	assertTrue( ring.insert("two", "2") );
	assertEquals( "2", ring._ring[Hash("two").str()] );

	assertTrue( ring.insert("foo", "bar") );
	assertEquals( "bar", ring._ring[Hash("foo").str()] );

	assertEquals( 3, ring._ring.size() );
}

TEST_CASE( "ConsistentHashRingTest/testErase", "[unit]" )
{
	TestableConsistentHashRing ring;
	assertFalse( ring.erase("nothing") );

	ring._ring[Hash("one").str()] = "one";
	ring._ring[Hash("two").str()] = "two";
	ring._ring[Hash("three").str()] = "three";
	assertEquals( 3, ring._ring.size() );

	assertTrue( ring.erase("one") );
	assertEquals( 2, ring._ring.size() );
	assertTrue( ring._ring.find(Hash("one").str()) == ring._ring.end() );
	assertFalse( ring.erase("one") );

	assertTrue( ring.erase("three") );
	assertEquals( 1, ring._ring.size() );
	assertEquals( "two", ring._ring[Hash("two").str()] );

	assertTrue( ring.erase("two") );
	assertEquals( 0, ring._ring.size() );
}

TEST_CASE( "ConsistentHashRingTest/testLocations.EmptyRing", "[unit]" )
{
	TestableConsistentHashRing ring;
	vector<string> locs = ring.locations("nobody", 3);
	assertTrue( locs.empty() );
}

TEST_CASE( "ConsistentHashRingTest/testLocations.Simple", "[unit]" )
{
	TestableConsistentHashRing ring;
	ring.insert("one", "one");
	ring.insert("two", "two");
	ring.insert("three", "three");
	assertEquals( 3, ring._ring.size() );

	vector<string> locs = ring.locations("one", 3);
	assertEquals( "one two three", turbo::str::join(locs) );

	locs = ring.locations("three", 5);
	assertEquals( "three one two", turbo::str::join(locs) );

	locs = ring.locations("two", 2);
	assertEquals( "two three", turbo::str::join(locs) );

	locs = ring.locations("foo", 3);
	assertEquals( "two three one", turbo::str::join(locs) );

	locs = ring.locations("bar", 1);
	assertEquals( "two", turbo::str::join(locs) );
}

TEST_CASE( "ConsistentHashRingTest/testNodeId", "[unit]" )
{
	TestableConsistentHashRing ring;
	assertEquals( "", ring.nodeId("nobody") );

	ring.insert("one", "one");
	ring.insert("two", "two");
	ring.insert("three", "three");
	assertEquals( 3, ring._ring.size() );

	assertEquals( Hash("one").str(), ring.nodeId("one") );
	assertEquals( Hash("two").str(), ring.nodeId("two") );
	assertEquals( Hash("three").str(), ring.nodeId("three") );
	assertEquals( Hash("two").str(), ring.nodeId("foo") );
}

