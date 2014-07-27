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
#include "wan_server/ConnectionWriteStream.h"

#include "util/CallHistory.h"
#include <string>

using std::string;

namespace {
	CallHistory _history;
}

TEST_CASE( "MirrorToPeerTest/testMirror_SelfIsNull", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	membership._self.reset();
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	WriteParams params("file",0,3,"v1",0);
	assertFalse( command.run(params, reader) );

	assertTrue( !params.outstream );
	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
	assertEquals( "", peers._history.calls() );
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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",0,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertFalse( !params.outstream );
	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3 v=v1 offset=0 source=me|,false)|write(0,contents,false)", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers, true);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",0,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3 v=v1 offset=0 source=me|,true)|write(0,contents,true)", writer->_history.calls() );
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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",0,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(bbb)", membership._history.calls() );
	assertEquals( "getWriter(bbb)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0 source=aaa|,false)|write(0,contents,false)", writer->_history.calls() );
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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",1,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(bbb)|lookup(ccc)", membership._history.calls() );
	assertEquals( "getWriter(ccc)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=3 n=3 v=v1 offset=0|,false)|write(0,contents,false)", writer->_history.calls() );
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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",0,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3 v=v1 offset=0 source=ccc|,false)|write(0,contents,false)", writer->_history.calls() );
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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "getWriter(ccc)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=3 n=3 v=v1 offset=0|,false)|write(0,contents,false)", writer->_history.calls() );
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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",3,3,"v1",0);
	assertFalse( command.run(params, reader) );

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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",3,4,"v1",0);
	assertFalse( command.run(params, reader) );

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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",1,2,"v1",0);
	params.source = "bbb";
	assertTrue( command.run(params, reader) );

	assertEquals( "locations(file,2)", ring._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "getWriter(ccc)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=3 n=2 v=v1 offset=0 source=bbb|,false)"
				  "|write(0,contents,false)", writer->_history.calls() );
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
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",0,3,"v1",0);
	params.isComplete = true;
	assertTrue( command.run(params, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3 v=v1 offset=0 source=me|,false)|write(0,contents,false)|write(0,,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "MirrorToPeerTest/testNoWriter", "[unit]" )
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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	// nope!

	WriteParams params("file",0,3,"v1",0);
	assertFalse( command.run(params, reader) );

	assertTrue( !params.outstream );
	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
}

TEST_CASE( "MirrorToPeerTest/testMultiplePackets", "[unit]" )
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
	MirrorToPeer command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",0,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertFalse( !params.outstream );
	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3 v=v1 offset=0 source=me|,false)|write(0,contents,false)", writer->_history.calls() );

	writer->_history.clear();
	reader = store.read("dummy", "version");
	assertTrue( command.run(params, reader) );
	assertEquals( "locations(file,3)", ring._history.calls() ); // no change
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,contents,false)", writer->_history.calls() );

	writer->_history.clear();
	reader = store.read("dummy", "version");
	params.isComplete = true;
	assertTrue( command.run(params, reader) );
	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "write(0,contents,false)|write(0,,false)|flush(false)", writer->_history.calls() );
}
