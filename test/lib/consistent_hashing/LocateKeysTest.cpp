/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "LocateKeys.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "serialize/StringUtil.h"

TEST_CASE( "LocateKeysTest/testLocations", "[unit]" )
{
	MockHashRing ring;
	ring._workers = {"foo", "bar"};
	MockMembership membership;

	LocateKeys locator(ring, membership);
	assertEquals( "foo bar", StringUtil::join(locator.locations("myfile", 5)) );
	assertEquals( "locations(myfile,5)", ring._history.calls() );
}

TEST_CASE( "LocateKeysTest/testContainsSelf", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;

	LocateKeys locator(ring, membership);

	assertTrue( locator.containsSelf({"me"}) );
	assertEquals( "containsSelf(me)", membership._history.calls() );
}

TEST_CASE( "LocateKeysTest/testKeyIsMine", "[unit]" )
{
	MockHashRing ring;
	ring._workers = {"me", "bar"};
	MockMembership membership;

	LocateKeys locator(ring, membership);

	assertTrue( locator.keyIsMine("myfile", 5) );
	assertEquals( "locations(myfile,5)", ring._history.calls() );
	assertEquals( "containsSelf(me|bar)", membership._history.calls() );
}

TEST_CASE( "LocateKeysTest/testKeyIsMine.Nope", "[unit]" )
{
	MockHashRing ring;
	ring._workers = {"foo", "bar"};
	MockMembership membership;
	membership._self.reset();

	LocateKeys locator(ring, membership);

	assertFalse( locator.keyIsMine("myfile", 5) );
	assertEquals( "locations(myfile,5)", ring._history.calls() );
	assertEquals( "containsSelf(foo|bar)", membership._history.calls() );
}

TEST_CASE( "LocateKeysTest/testKeyIsMine.NoLocs", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;

	LocateKeys locator(ring, membership);

	assertTrue( locator.keyIsMine("myfile", 5) );
	assertEquals( "locations(myfile,5)", ring._history.calls() );
	assertEquals( "", membership._history.calls() );
}
