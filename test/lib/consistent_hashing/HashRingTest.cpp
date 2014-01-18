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
	assertEquals( "ECjsTyiCEvexJ1xX4Bjs4MkzK0epOKO6", HashRing::hash("foo") );
	assertEquals( "MeNRjt+Y9fNwNBdWLdvc8TyTj8EES9Lw", HashRing::hash("bar") );
	assertEquals( "2Ok5EtCP5L0BO2S1lrG9C1U4x/a0mU03", HashRing::hash("one") );
	assertEquals( "QWvKlLwjsuwwU3DcZdYtCBBWoLu0a8Oz", HashRing::hash("two") );
	assertEquals( "1jN1njgiq71DV2buECVBL5uJ+C4zTUZX", HashRing::hash("three") );
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
	assertEquals( "one", StringUtil::stlJoin(locs) );

	locs = ring.locationsFromHash(HashRing::hash("two"), 1);
	assertEquals( "two", StringUtil::stlJoin(locs) );

	locs = ring.locationsFromHash(HashRing::hash("three"), 1);
	assertEquals( "three", StringUtil::stlJoin(locs) );
}

TEST_CASE( "HashRingTest/testLookupSection", "[unit]" )
{
	HashRing ring;
	assertEquals( "", ring.section("foo") );
	assertEquals( "", ring.section("bar") );

	vector<string> locs;
	assertEquals( "", ring.lookup("foo", locs, 1) );
	assertEquals( "", StringUtil::stlJoin(locs) );

	ring.addWorker("one");
	ring.addWorker("two");
	ring.addWorker("three");

	assertEquals( HashRing::hash("one"), ring.section("one") );
	assertEquals( HashRing::hash("two"), ring.section("two") );
	assertEquals( HashRing::hash("three"), ring.section("three") );

	assertEquals( HashRing::hash("one"), ring.lookup("one", locs, 1) );
	assertEquals( "one", StringUtil::stlJoin(locs) );

	locs.clear();
	assertEquals( HashRing::hash("two"), ring.section("foo") );
	assertEquals( HashRing::hash("two"), ring.lookup("foo", locs, 3) );
	assertEquals( "two three one", StringUtil::stlJoin(locs) );
}

