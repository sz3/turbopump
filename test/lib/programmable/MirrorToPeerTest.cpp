/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MirrorToPeer.h"

#include "actions/WriteParams.h"
#include "membership/Peer.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"

#include "util/CallHistory.h"
#include <memory>
#include <string>
using std::shared_ptr;
using std::string;

TEST_CASE( "MirrorToPeerTest/testMirror_SelfIsNull", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership._self.reset();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",0,3,"v1",0);
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
	assertTrue( !peer );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SelfNotInList", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",0,3,"v1",0);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "aaa", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SelfNotInList_EnsureDelivery", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",0,3,"v1",0);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "aaa", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SkipSource", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._self = membership.lookup("aaa");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",0,3,"v1",0);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(bbb)", membership._history.calls() );
	assertEquals( "bbb", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SkipSelf", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._self = membership.lookup("bbb");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",1,3,"v1",0);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(bbb)|lookup(ccc)", membership._history.calls() );
	assertEquals( "ccc", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_SelfLaterInList", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._self = membership.lookup("ccc");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",0,3,"v1",0);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "aaa", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_LaterIndex", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",2,3,"v1",0);
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "ccc", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_Done", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",3,3,"v1",0);
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
}

TEST_CASE( "MirrorToPeerTest/testMirror_NoAcceptablePeers", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._self = membership.lookup("ddd");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",3,4,"v1",0);
	shared_ptr<Peer> peer;
	assertFalse( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,4)", ring._history.calls() );
	assertEquals( "self()|lookup(ddd)", membership._history.calls() );
}

TEST_CASE( "MirrorToPeerTest/testMirror_AlreadyHitSource", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._self = membership.lookup("ddd");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",1,2,"v1",0);
	params.source = "bbb";
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,2)", ring._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "ccc", peer->uid );
}

TEST_CASE( "MirrorToPeerTest/testMirror_Fin", "[unit]" )
{
	MockHashRing ring;
	ring._workers.push_back("aaa");
	ring._workers.push_back("bbb");
	ring._workers.push_back("ccc");
	ring._workers.push_back("ddd");
	MockMembership membership;
	membership.addIp("aaa", "aaa");
	membership.addIp("bbb", "bbb");
	membership.addIp("ccc", "ccc");
	membership.addIp("ddd", "ddd");
	membership._history.clear();
	MirrorToPeer command(ring, membership);

	WriteParams params("file",0,3,"v1",0);
	params.isComplete = true;
	shared_ptr<Peer> peer;
	assertTrue( command.chooseMirror(params, peer) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "aaa", peer->uid );
}
