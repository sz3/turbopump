/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MirrorToPeer.h"

#include "common/KeyMetadata.h"
#include "data_store/IDataStoreReader.h"
#include "membership/Peer.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockDataStore.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockMessageSender.h"
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
	membership._self.reset( new Peer("me") );
	membership._history.clear();
	MockMessageSender messenger;
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, messenger, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run(KeyMetadata({"file",0,3}), reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3 source=me|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	membership._self = membership.lookup("aaa");
	membership._history.clear();
	MockMessageSender messenger;
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, messenger, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run({"file",0,3}, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)|lookup(bbb)", membership._history.calls() );
	assertEquals( "getWriter(bbb)", peers._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	MockMessageSender messenger;
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, messenger, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run({"file",0,3}, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(aaa)", membership._history.calls() );
	assertEquals( "getWriter(aaa)", peers._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "write(0,write|name=file i=1 n=3|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	MockMessageSender messenger;
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, messenger, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( command.run({"file",2,3}, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(ccc)", membership._history.calls() );
	assertEquals( "getWriter(ccc)", peers._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "write(0,write|name=file i=3 n=3|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
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
	MockMessageSender messenger;
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, messenger, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertFalse( command.run({"file",3,3}, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()", membership._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", writer->_history.calls() );
}

// same as Done, but with an extra operation... dropKey()
TEST_CASE( "MirrorToPeerTest/testMirror_Done_DropExtraMirror", "[unit]" )
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
	membership.addIp("source", "source");
	membership._history.clear();
	MockMessageSender messenger;
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, messenger, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	KeyMetadata md("file", 3, 3);
	md.source = "source";
	assertFalse( command.run(md, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(source)", membership._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "dropKey(source,file)", messenger._history.calls() );
	assertEquals( "", writer->_history.calls() );
}

TEST_CASE( "MirrorToPeerTest/testMirror_Done_DropExtraMirrorFails", "[unit]" )
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
	// mirror "source" isn't a valid member
	membership._history.clear();
	MockMessageSender messenger;
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, messenger, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	KeyMetadata md("file", 3, 3);
	md.source = "source";
	assertFalse( command.run(md, reader) );

	assertEquals( "locations(file,3)", ring._history.calls() );
	assertEquals( "self()|lookup(source)", membership._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", messenger._history.calls() );
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
	MockMessageSender messenger;
	MockPeerTracker peers;
	MirrorToPeer command(ring, membership, messenger, peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertFalse( command.run({"file",3,4}, reader) );

	assertEquals( "locations(file,4)", ring._history.calls() );
	assertEquals( "self()|lookup(ddd)", membership._history.calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", messenger._history.calls() );
	assertEquals( "", writer->_history.calls() );
}


