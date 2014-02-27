/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MirrorToPeer.h"

#include "actions/WriteParams.h"
#include "data_store/IDataStoreReader.h"
#include "membership/Peer.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockDataStore.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockPeerTracker.h"

#include "util/CallHistory.h"
#include <string>

using std::string;

namespace {
	CallHistory _history;
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run(WriteParams({"file",0,3,"v1"}), reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3 v=v1 source=me|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run(WriteParams({"file",0,3,"v1"}), reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(bbb)", membership._history.calls() );
	assertEquals( "getWriter(bbb)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 source=aaa|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run(WriteParams({"file",1,3,"v1"}), reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(bbb)|lookup(ccc)", membership._history.calls() );
	assertEquals( "getWriter(ccc)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=3 n=3 v=v1|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run(WriteParams({"file",0,3,"v1"}), reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3 v=v1 source=ccc|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run(WriteParams({"file",2,3,"v1"}), reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "getWriter(ccc)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=3 n=3 v=v1|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertFalse( command.run(WriteParams({"file",3,3,"v1"}), reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertFalse( command.run(WriteParams({"file",3,4,"v1"}), reader) );

	assertEquals( "locations(file,4)", ring._history.calls() );
	assertEquals( "self()|lookup(ddd)", membership._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params({"file",1,2,"v1"});
	params.source = "bbb";
	assertTrue( command.run(params, reader) );

	assertEquals( "locations(file,2)", ring._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "getWriter(ccc)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=3 n=2 v=v1 source=bbb|)"
				  "|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
}


