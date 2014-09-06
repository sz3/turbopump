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
#include "mock/MockWriteSupervisor.h"

#include <string>
using std::string;

TEST_CASE( "ChainWriteTest/testBasic", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MockWriteSupervisor supervisor;
	ChainWrite<MockMirrorToPeer> command(ring, membership, supervisor, true);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	supervisor._writer.reset(new MockBufferedConnectionWriter());

	WriteParams params("file",0,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertFalse( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "open(peerid,file,1)|store(file|v1|false,8)", supervisor._history.calls() );
}

TEST_CASE( "ChainWriteTest/testChooseMirrorFails", "[unit]" )
{
	MockMirrorToPeer::failOnce();

	MockHashRing ring;
	MockMembership membership;
	MockWriteSupervisor supervisor;
	ChainWrite<MockMirrorToPeer> command(ring, membership, supervisor, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	supervisor._writer.reset(new MockBufferedConnectionWriter());

	WriteParams params("file",0,3,"v1",0);
	assertFalse( command.run(params, reader) );

	assertTrue( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "", supervisor._history.calls() );
}

TEST_CASE( "ChainWriteTest/testNoWriter", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MockWriteSupervisor supervisor;
	ChainWrite<MockMirrorToPeer> command(ring, membership, supervisor, false);

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
	assertEquals( "open(peerid,file,0)", supervisor._history.calls() );
}

TEST_CASE( "ChainWriteTest/testMultiplePackets", "[unit]" )
{
	MockHashRing ring;
	MockMembership membership;
	MockWriteSupervisor supervisor;
	ChainWrite<MockMirrorToPeer> command(ring, membership, supervisor, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	supervisor._writer.reset(new MockBufferedConnectionWriter());

	WriteParams params("file",0,3,"v1",0);
	assertTrue( command.run(params, reader) );

	assertFalse( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "open(peerid,file,0)|store(file|v1|false,8)", supervisor._history.calls() );

	supervisor._history.clear();

	reader = store.read("dummy", "version");
	assertTrue( command.run(params, reader) );
	assertEquals( "", MockMirrorToPeer::calls() );
	assertEquals( "store(file|v1|false,8)", supervisor._history.calls() );

	supervisor._history.clear();

	reader = store.read("dummy", "version");
	params.isComplete = true;
	assertTrue( command.run(params, reader) );
	assertEquals( "", MockMirrorToPeer::calls() );
	assertEquals( "store(file|v1|true,8)", supervisor._history.calls() );
}
