/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "RandomizedMirrorToPeer.h"
#include "api/WriteInstructions.h"
#include "membership/Peer.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockKnownPeers.h"

#include <memory>
#include <string>
using std::shared_ptr;
using std::string;

TEST_CASE( "RandomizedMirrorToPeerTest/testSuccess", "[unit]" )
{
	MockLocateKeys locator;
	MockKnownPeers membership;
	membership.update("dude", {"1.2.3.4"});
	membership._history.clear();
	RandomizedMirrorToPeer command(locator, membership);

	WriteInstructions params("file", "v1", 123, 456);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "randomPeer()", membership._history.calls() );
	assertEquals( "dude", peer->uid );
	assertEquals( "", locator._history.calls() );
}

TEST_CASE( "RandomizedMirrorToPeerTest/testDone", "[unit]" )
{
	MockLocateKeys locator;
	MockKnownPeers membership;
	membership.update("dude", {"1.2.3.4"});
	membership._history.clear();
	RandomizedMirrorToPeer command(locator, membership);

	WriteInstructions params("file", "v1", 3, 3);
	params.mirror = 3;
	params.copies = 3;
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "", membership._history.calls() );
	assertTrue( !peer );
}

TEST_CASE( "RandomizedMirrorToPeerTest/testBadPeer", "[unit]" )
{
	MockLocateKeys locator;
	MockKnownPeers membership;
	membership._history.clear();
	RandomizedMirrorToPeer command(locator, membership);

	WriteInstructions params("file", "v1", 123, 456);
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "randomPeer()", membership._history.calls() );
	assertTrue( !peer );
}
