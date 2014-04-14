/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "VectorClock.h"
#include <deque>

TEST_CASE( "VectorClockTest/testToString", "[unit]" )
{
	VectorClock version;
	version.increment("one");
	version.increment("two");
	version.increment("two");
	version.increment("three");
	version.increment("three");
	version.increment("three");

	assertEquals("3,three:3,two:2,one:1", version.toString());
}

TEST_CASE( "VectorClockTest/testFromString", "[unit]" )
{
	VectorClock version;
	assertTrue( version.fromString("3,one:1,two:2,three:3") );

	std::deque<VectorClock::clock> clocks = version.clocks();
	assertEquals( 3, clocks.size() );
	assertEquals( "one", clocks[0].key );
	assertEquals( 1, clocks[0].count );
	assertEquals( "two", clocks[1].key );
	assertEquals( 2, clocks[1].count );
	assertEquals( "three", clocks[2].key );
	assertEquals( 3, clocks[2].count );
}

TEST_CASE( "VectorClockTest/testSerialize", "[unit]" )
{
	VectorClock version;
	version.increment("foo");
	version.increment("foo");
	version.increment("bar");

	VectorClock another;
	assertTrue( another.fromString(version.toString()) );

	std::deque<VectorClock::clock> clocks = another.clocks();
	assertEquals( 2, clocks.size() );
	assertEquals( "bar", clocks[0].key );
	assertEquals( 1, clocks[0].count );
	assertEquals( "foo", clocks[1].key );
	assertEquals( 2, clocks[1].count );
}

TEST_CASE( "VectorClockTest/testMarkDeleted", "[unit]" )
{
	{
		VectorClock version;
		assertFalse( version.isDeleted() );
	}

	{
		VectorClock version;
		version.markDeleted();
		assertEquals("1,delete:1", version.toString());
		assertTrue( version.isDeleted() );
	}

	{
		VectorClock version;
		version.increment("foo");
		version.markDeleted();
		assertEquals("2,delete:1,foo:1", version.toString());
		assertTrue( version.isDeleted() );
	}

	{
		VectorClock version;
		version.increment("foo");
		version.markDeleted();
		version.increment("foo");
		assertEquals("2,foo:2,delete:1", version.toString());
		assertFalse( version.isDeleted() );
	}
}
