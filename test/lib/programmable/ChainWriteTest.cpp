/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ChainWrite.h"

#include "actions/WriteParams.h"
#include "data_store/IDataStoreReader.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockDataStore.h"
#include "mock/MockHashRing.h"
#include "mock/MockMembership.h"
#include "mock/MockMirrorToPeer.h"
#include "mock/MockPeerTracker.h"

#include <string>
using std::string;

TEST_CASE( "ChainWriteTest/testBasic", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MockPeerTracker peers;
	ChainWrite<MockMirrorToPeer> command(ring, membership, peers, false);

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
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "getWriter(peerid)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=0 n=3 v=v1 offset=0|,false)|write(0,contents,false)", writer->_history.calls() );
}

TEST_CASE( "ChainWriteTest/testChooseMirrorFails", "[unit]" )
{
	MockMirrorToPeer::failOnce();

	MockHashRing ring;
	MockMembership membership;
	MockPeerTracker peers;
	ChainWrite<MockMirrorToPeer> command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",0,3,"v1",0);
	assertFalse( command.run(params, reader) );

	assertTrue( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "", writer->_history.calls() );
}

TEST_CASE( "ChainWriteTest/testNoWriter", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MockPeerTracker peers;
	ChainWrite<MockMirrorToPeer> command(ring, membership, peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	// nope!

	WriteParams params("file",0,3,"v1",0);
	assertFalse( command.run(params, reader) );

	assertTrue( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "getWriter(peerid)", peers._history.calls() );
}

TEST_CASE( "ChainWriteTest/testMultiplePackets", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MockPeerTracker peers;
	ChainWrite<MockMirrorToPeer> command(ring, membership, peers, false);

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
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "getWriter(peerid)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=0 n=3 v=v1 offset=0|,false)|write(0,contents,false)", writer->_history.calls() );

	peers._history.clear();
	writer->_history.clear();

	reader = store.read("dummy", "version");
	assertTrue( command.run(params, reader) );
	assertEquals( "", MockMirrorToPeer::calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "write(0,contents,false)", writer->_history.calls() );

	writer->_history.clear();

	reader = store.read("dummy", "version");
	params.isComplete = true;
	assertTrue( command.run(params, reader) );
	assertEquals( "", MockMirrorToPeer::calls() );
	assertEquals( "", peers._history.calls() );
	assertEquals( "write(0,contents,false)|write(0,,false)|flush(false)", writer->_history.calls() );
}
