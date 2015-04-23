/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "VectorClock.h"
#include "common/WallClock.h"
#include <deque>

TEST_CASE( "VectorClockTest/testToString", "[unit]" )
{
	VectorClock version;
	WallClock().freeze(WallClock::MAGIC_NUMBER);

	SECTION( "one" )
	{
		WallClock().freeze(11);
		version.increment("one");

		WallClock().freeze(22);
		version.increment("two");
		version.increment("two");
		version.increment("three");
		version.increment("three");

		WallClock().freeze(WallClock::MAGIC_NUMBER);
		version.increment("three");

		assertEquals("3,three.UNIXSECONDS=,two.5W000000000=.1,one.2k000000000=", version.toString());
	}

	SECTION( "two" )
	{
		version.increment("one");
		version.increment("two");
		version.increment("two");
		version.increment("three");
		version.increment("three");
		version.increment("three");

		assertEquals("3,three.UNIXSECONDS=.2,two.UNIXSECONDS=.1,one.UNIXSECONDS=", version.toString());
	}
}

TEST_CASE( "VectorClockTest/testFromString", "[unit]" )
{
	VectorClock version;
	assertTrue( version.fromString("3,one.0G000000000=,two.0W000000000=.1,three.UNIXSECONDS=") );

	std::deque<VectorClock::clock> clocks = version.clocks();
	assertEquals( 3, clocks.size() );
	assertEquals( "one", clocks[0].key );
	assertEquals( 1, clocks[0].time );
	assertEquals( 0, clocks[0].count );
	assertEquals( "two", clocks[1].key );
	assertEquals( 2, clocks[1].time );
	assertEquals( 1, clocks[1].count );
	assertEquals( "three", clocks[2].key );
	assertEquals( WallClock::MAGIC_NUMBER, clocks[2].time );
	assertEquals( 0, clocks[2].count );
}

TEST_CASE( "VectorClockTest/testFromString.Bad", "[unit]" )
{
	VectorClock version;
	assertTrue( version.fromString("2,.1,1.UNIXSECONDS=.5") );

	std::deque<VectorClock::clock> clocks = version.clocks();
	assertEquals( 2, clocks.size() );
	assertEquals( "", clocks[0].key );
	assertEquals( 0, clocks[0].time );
	assertEquals( 0, clocks[0].count );
	assertEquals( "1", clocks[1].key );
	assertEquals( WallClock::MAGIC_NUMBER, clocks[1].time );
	assertEquals( 5, clocks[1].count );
}

TEST_CASE( "VectorClockTest/testAssignMerge", "[unit]" )
{
	WallClock().freeze(WallClock::MAGIC_NUMBER);

	VectorClock version;
	version.increment("foo");
	version.increment("foo");
	version.increment("bar");

	VectorClock another;
	assertEquals( "0", another.toString() );
	another = version;
	assertEquals( "2,bar.UNIXSECONDS=,foo.UNIXSECONDS=.1", another.toString() );

	VectorClock onemore;
	assertEquals( "0", onemore.toString() );
	onemore.merge(version);
	assertEquals( "2,bar.UNIXSECONDS=,foo.UNIXSECONDS=.1", onemore.toString() );
}

TEST_CASE( "VectorClockTest/testSerialize", "[unit]" )
{
	WallClock().freeze(12345);

	VectorClock version;
	version.increment("foo");
	version.increment("foo");
	version.increment("bar");

	VectorClock another;
	assertTrue( another.fromString(version.toString()) );

	std::deque<VectorClock::clock> clocks = another.clocks();
	assertEquals( 2, clocks.size() );
	assertEquals( "bar", clocks[0].key );
	assertEquals( 12345, clocks[0].time );
	assertEquals( 0, clocks[0].count );
	assertEquals( "foo", clocks[1].key );
	assertEquals( 12345, clocks[1].time );
	assertEquals( 1, clocks[1].count );
}

TEST_CASE( "VectorClockTest/testMarkDeleted", "[unit]" )
{
	WallClock().freeze(WallClock::MAGIC_NUMBER);

	{
		VectorClock version;
		assertFalse( version.isDeleted() );
	}

	{
		VectorClock version;
		version.markDeleted();
		assertEquals("1,delete.UNIXSECONDS=", version.toString());
		assertTrue( version.isDeleted() );
	}

	{
		VectorClock version;
		version.increment("foo");
		version.markDeleted();
		assertEquals("2,delete.UNIXSECONDS=,foo.UNIXSECONDS=", version.toString());
		assertTrue( version.isDeleted() );
	}

	{
		VectorClock version;
		version.increment("foo");
		version.markDeleted();
		version.increment("foo");
		assertEquals("2,foo.UNIXSECONDS=.1,delete.UNIXSECONDS=", version.toString());
		assertFalse( version.isDeleted() );
	}
}
