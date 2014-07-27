/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "RandomizedMirrorToPeer.h"
#include "actions/WriteParams.h"
#include "membership/Peer.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"

#include <memory>
#include <string>
using std::shared_ptr;
using std::string;

TEST_CASE( "RandomizedMirrorToPeerTest/testSuccess", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership.addIp("1.2.3.4", "dude");
	membership._history.clear();
	RandomizedMirrorToPeer command(ring, membership);

	WriteParams params("file", 123, 456, "v1", 0);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "randomPeer()", membership._history.calls() );
	assertEquals( "dude", peer->uid );
	assertEquals( "", ring._history.calls() );
}

TEST_CASE( "RandomizedMirrorToPeerTest/testDone", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership.addIp("1.2.3.4", "dude");
	membership._history.clear();
	RandomizedMirrorToPeer command(ring, membership);

	WriteParams params("file", 123, 456, "v1", 0);
	params.mirror = 3;
	params.totalCopies = 3;
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "", membership._history.calls() );
	assertTrue( !peer );
}

TEST_CASE( "RandomizedMirrorToPeerTest/testBadPeer", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership._history.clear();
	RandomizedMirrorToPeer command(ring, membership);

	WriteParams params("file", 123, 456, "v1", 0);
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "randomPeer()", membership._history.calls() );
	assertTrue( !peer );
}
