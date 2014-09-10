/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ConsistentHashRing.h"
#include "Hash.h"
#include "serialize/StringUtil.h"
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
	assertEquals( "1", ring._ring[Hash("one").base64()] );

	assertTrue( ring.insert("two", "2") );
	assertEquals( "2", ring._ring[Hash("two").base64()] );

	assertTrue( ring.insert("foo", "bar") );
	assertEquals( "bar", ring._ring[Hash("foo").base64()] );

	assertEquals( 3, ring._ring.size() );
}

TEST_CASE( "ConsistentHashRingTest/testErase", "[unit]" )
{
	TestableConsistentHashRing ring;
	assertFalse( ring.erase("nothing") );

	ring._ring[Hash("one").base64()] = "one";
	ring._ring[Hash("two").base64()] = "two";
	ring._ring[Hash("three").base64()] = "three";
	assertEquals( 3, ring._ring.size() );

	assertTrue( ring.erase("one") );
	assertEquals( 2, ring._ring.size() );
	assertTrue( ring._ring.find(Hash("one").base64()) == ring._ring.end() );
	assertFalse( ring.erase("one") );

	assertTrue( ring.erase("three") );
	assertEquals( 1, ring._ring.size() );
	assertEquals( "two", ring._ring[Hash("two").base64()] );

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
	assertEquals( "one two three", StringUtil::join(locs) );

	locs = ring.locations("three", 5);
	assertEquals( "three one two", StringUtil::join(locs) );

	locs = ring.locations("two", 2);
	assertEquals( "two three", StringUtil::join(locs) );

	locs = ring.locations("foo", 3);
	assertEquals( "two three one", StringUtil::join(locs) );

	locs = ring.locations("bar", 1);
	assertEquals( "two", StringUtil::join(locs) );
}

TEST_CASE( "ConsistentHashRingTest/testNodeId", "[unit]" )
{
	TestableConsistentHashRing ring;
	assertEquals( "", ring.nodeId("nobody") );

	ring.insert("one", "one");
	ring.insert("two", "two");
	ring.insert("three", "three");
	assertEquals( 3, ring._ring.size() );

	assertEquals( Hash("one").base64(), ring.nodeId("one") );
	assertEquals( Hash("two").base64(), ring.nodeId("two") );
	assertEquals( Hash("three").base64(), ring.nodeId("three") );
	assertEquals( Hash("two").base64(), ring.nodeId("foo") );
}

