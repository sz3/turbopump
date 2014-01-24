/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "HashRing.h"
#include "serialize/StringUtil.h"
#include <vector>
using std::string;
using std::vector;

TEST_CASE( "HashRingTest/testHash", "[unit]" )
{
	// Tiger -> base64
	assertEquals( "20XgHmW02jSl7plLs/XgsAYn8oSdC8Cu", HashRing::hash("foo") );
	assertEquals( "ASBFXhyMxTBkB/RK9RjQwHmHXw22Gx9k", HashRing::hash("bar") );
	assertEquals( "qCYt2h0Dt9o/CqGpZf4x0pIslzOoaIor", HashRing::hash("one") );
	assertEquals( "EKj8Z9kXgikkIr1QNRMh0//Kc9ioOwCn", HashRing::hash("two") );
	assertEquals( "pXBpbXUWevp1JqPi20J/9ti7y0snHINL", HashRing::hash("three") );
}

TEST_CASE( "HashRingTest/testLocations", "[unit]" )
{
	HashRing ring;
	assertEquals( 0, ring.size() );

	vector<string> locs = ring.locations("foo");
	assertEquals(0, locs.size() );

	ring.addWorker("one");
	ring.addWorker("two");
	ring.addWorker("three");
	assertEquals( 3, ring.size() );

	locs = ring.locations("foo");
	assertEquals( 3, locs.size() );
}

TEST_CASE( "HashRingTest/testPredictability", "[unit]" )
{
	HashRing ring;
	ring.addWorker("one");
	ring.addWorker("two");
	ring.addWorker("three");

	vector<string> locs = ring.locations("one", 1);
	assertEquals( 1, locs.size() );
	assertEquals( "one", locs.front() );

	locs = ring.locations("two", 1);
	assertEquals( 1, locs.size() );
	assertEquals( "two", locs.front() );

	locs = ring.locations("three", 1);
	assertEquals( 1, locs.size() );
	assertEquals( "three", locs.front() );
}

TEST_CASE( "HashRingTest/testLocationsFromHash", "[unit]" )
{
	HashRing ring;

	vector<string> locs = ring.locationsFromHash("foo", 3);
	assertEquals( 0, locs.size() );

	ring.addWorker("one");
	ring.addWorker("two");
	ring.addWorker("three");

	locs = ring.locationsFromHash(HashRing::hash("one"), 1);
	assertEquals( "one", StringUtil::join(locs) );

	locs = ring.locationsFromHash(HashRing::hash("two"), 1);
	assertEquals( "two", StringUtil::join(locs) );

	locs = ring.locationsFromHash(HashRing::hash("three"), 1);
	assertEquals( "three", StringUtil::join(locs) );
}

TEST_CASE( "HashRingTest/testLookupSection", "[unit]" )
{
	HashRing ring;
	assertEquals( "", ring.section("foo") );
	assertEquals( "", ring.section("bar") );

	vector<string> locs;
	assertEquals( "", ring.lookup("foo", locs, 1) );
	assertEquals( "", StringUtil::join(locs) );

	ring.addWorker("one");
	ring.addWorker("two");
	ring.addWorker("three");

	assertEquals( HashRing::hash("one"), ring.section("one") );
	assertEquals( HashRing::hash("two"), ring.section("two") );
	assertEquals( HashRing::hash("three"), ring.section("three") );

	assertEquals( HashRing::hash("one"), ring.lookup("one", locs, 1) );
	assertEquals( "one", StringUtil::join(locs) );

	locs.clear();
	assertEquals( HashRing::hash("two"), ring.section("foo") );
	assertEquals( HashRing::hash("two"), ring.lookup("foo", locs, 3) );
	assertEquals( "two three one", StringUtil::join(locs) );
}

