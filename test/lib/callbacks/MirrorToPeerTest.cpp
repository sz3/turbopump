/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MirrorToPeer.h"

#include "api/WriteInstructions.h"
#include "membership/Peer.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockKnownPeers.h"

#include "util/CallHistory.h"
#include <memory>
#include <string>
using std::shared_ptr;
using std::string;

TEST_CASE( "MirrorToPeerTest/testMirror_SelfIsNull", "[unit]" )
{
	MockLocateKeys locator;
	MockKnownPeers membership;
	membership._self.reset();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",0,3);
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", locator._history.calls() );
	assertEquals( "self()", membership._history.calls() );
	assertTrue( !peer );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SelfNotInList", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",0,3);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", locator._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "aaa", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SelfNotInList_EnsureDelivery", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",0,3);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", locator._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "aaa", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SkipSource", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._self = membership.lookup("aaa");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",0,3);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", locator._history.calls() );
	assertEquals( "self()|lookup(bbb)", membership._history.calls() );
	assertEquals( "bbb", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SkipSelf", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._self = membership.lookup("bbb");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",1,3);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", locator._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "ccc", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SelfLaterInList", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._self = membership.lookup("ccc");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",0,3);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", locator._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "aaa", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_LaterIndex", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",2,3);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", locator._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "ccc", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_Done", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",3,3);
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "", locator._history.calls() );
	assertEquals( "", membership._history.calls() );
}

TEST_CASE( "MirrorToPeerTest/testMirror_NoAcceptablePeers", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._self = membership.lookup("ddd");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",3,4);
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,4)", locator._history.calls() );
	assertEquals( "self()", membership._history.calls() );
}

TEST_CASE( "MirrorToPeerTest/testMirror_AlreadyHitSource", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._self = membership.lookup("ddd");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",1,2);
	params.source = "bbb";
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,2)", locator._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "ccc", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_Fin", "[unit]" )
{
	MockLocateKeys locator;
	locator._locations.push_back("aaa");
	locator._locations.push_back("bbb");
	locator._locations.push_back("ccc");
	locator._locations.push_back("ddd");
	MockKnownPeers membership;
	membership.update("aaa");
	membership.update("bbb");
	membership.update("ccc");
	membership.update("ddd");
	membership._history.clear();
	MirrorToPeer command(locator, membership);

	WriteInstructions params("file","v1",0,3);
	params.isComplete = true;
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", locator._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "aaa", peer->uid );
}
