/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "catch.hpp"

#include "HashRing.h"
#include <vector>
using std::vector;

TEST_CASE( "HashRingTest/testHash", "default" )
{
	// Tiger -> base64
	REQUIRE( "ECjsTyiCEvexJ1xX4Bjs4MkzK0epOKO6" == HashRing::hash("foo") );
	REQUIRE( "MeNRjt+Y9fNwNBdWLdvc8TyTj8EES9Lw" == HashRing::hash("bar") );
	REQUIRE( "2Ok5EtCP5L0BO2S1lrG9C1U4x/a0mU03" == HashRing::hash("one") );
	REQUIRE( "QWvKlLwjsuwwU3DcZdYtCBBWoLu0a8Oz" == HashRing::hash("two") );
	REQUIRE( "1jN1njgiq71DV2buECVBL5uJ+C4zTUZX" == HashRing::hash("three") );
}

TEST_CASE( "HashRingTest/testDefault", "default" )
{
	HashRing ring;
	REQUIRE( ring.size() == 0 );

	vector<string> locs = ring.lookup("foo");
	REQUIRE( locs.size() == 0 );

	ring.addWorker("one");
	ring.addWorker("two");
	ring.addWorker("three");
	REQUIRE( ring.size() == 3 );

	locs = ring.lookup("foo");
	REQUIRE( locs.size() == 3 );
}

TEST_CASE( "HashRingTest/testPredictability", "default" )
{
	HashRing ring;
	ring.addWorker("one");
	ring.addWorker("two");
	ring.addWorker("three");

	vector<string> locs = ring.lookup("one", 1);
	REQUIRE( locs.size() == 1 );
	REQUIRE( locs.front() == "one" );

	locs = ring.lookup("two", 1);
	REQUIRE( locs.size() == 1 );
	REQUIRE( locs.front() == "two" );

	locs = ring.lookup("three", 1);
	REQUIRE( locs.size() == 1 );
	REQUIRE( locs.front() == "three" );
}

