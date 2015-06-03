/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "LocateKeys.h"

#include "Hash.h"
#include "membership/Peer.h"
#include "mock/MockConsistentHashRing.h"
#include "mock/MockKnownPeers.h"
#include "serialize/str_join.h"

TEST_CASE( "LocateKeysTest/testLocations", "[unit]" )
{
	MockConsistentHashRing ring;
	ring._locations = {"foo", "bar"};
	MockKnownPeers membership;

	LocateKeys locator(ring, membership);
	assertEquals( "foo bar", turbo::str::join(locator.locations("myfile", 5)) );
	assertStringsEqual( "locations(" + Hash("myfile").base64() + ",5)", ring._history.calls() );
}

TEST_CASE( "LocateKeysTest/testContainsSelf", "[unit]" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	membership._self.reset(new Peer("me"));

	LocateKeys locator(ring, membership);

	assertTrue( locator.containsSelf({"me"}) );
	assertEquals( "self()", membership._history.calls() );

	assertFalse( locator.containsSelf({}) );
	assertFalse( locator.containsSelf({"foo"}) );
	assertFalse( locator.containsSelf({"foo", "bar"}) );
	assertTrue( locator.containsSelf({"foo", "me"}) );
	assertTrue( locator.containsSelf({"foo", "me", "bar"}) );
	assertTrue( locator.containsSelf({"me", "bar"}) );
}

TEST_CASE( "LocateKeysTest/testKeyIsMine", "[unit]" )
{
	MockConsistentHashRing ring;
	ring._locations = {"me", "bar"};
	MockKnownPeers membership;
	membership._self.reset(new Peer("me"));

	LocateKeys locator(ring, membership);

	assertTrue( locator.keyIsMine("myfile", 5) );
	assertStringsEqual( "locations(" + Hash("myfile").base64() + ",5)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
}

TEST_CASE( "LocateKeysTest/testKeyIsMine.Nope", "[unit]" )
{
	MockConsistentHashRing ring;
	ring._locations = {"foo", "bar"};
	MockKnownPeers membership;
	membership._self.reset(new Peer("me"));

	LocateKeys locator(ring, membership);

	assertFalse( locator.keyIsMine("myfile", 5) );
	assertStringsEqual( "locations(" + Hash("myfile").base64() + ",5)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
}

TEST_CASE( "LocateKeysTest/testKeyIsMine.NoLocs", "[unit]" )
{
	MockConsistentHashRing ring;
	MockKnownPeers membership;
	membership._self.reset(new Peer("me"));

	LocateKeys locator(ring, membership);

	assertTrue( locator.keyIsMine("myfile", 5) );
	assertStringsEqual( "locations(" + Hash("myfile").base64() + ",5)", ring._history.calls() );
	assertEquals( "", membership._history.calls() );
}
