/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "ChainWrite.h"

#include "api/WriteInstructions.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockKnownPeers.h"
#include "mock/MockMirrorToPeer.h"
#include "mock/MockWriteSupervisor.h"
#include "storage/readstream.h"
#include "storage/StringReader.h"

#include "socket/MockSocketWriter.h"
#include <string>
using std::string;

TEST_CASE( "ChainWriteTest/testBasic", "[unit]" )
{
	MockLocateKeys locator;
	MockKnownPeers membership;
	MockWriteSupervisor supervisor;
	ChainWrite<MockMirrorToPeer> command(locator, membership, supervisor, true);

	// input
	readstream contents( new StringReader("contents"), KeyMetadata() );

	// output
	supervisor._writer.reset(new MockSocketWriter());

	WriteInstructions params("file","v1",0,3);
	assertTrue( command.run(params, contents) );

	assertFalse( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "open(peerid,file,1)|store(file|v1|false,8,0)", supervisor._history.calls() );
}

TEST_CASE( "ChainWriteTest/testChooseMirrorFails", "[unit]" )
{
	MockMirrorToPeer::failOnce();

	MockLocateKeys locator;
	MockKnownPeers membership;
	MockWriteSupervisor supervisor;
	ChainWrite<MockMirrorToPeer> command(locator, membership, supervisor, false);

	// input
	readstream contents( new StringReader("contents"), KeyMetadata() );

	// output
	supervisor._writer.reset(new MockSocketWriter());

	WriteInstructions params("file","v1",0,3);
	assertFalse( command.run(params, contents) );

	assertTrue( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "", supervisor._history.calls() );
}

TEST_CASE( "ChainWriteTest/testNoWriter", "[unit]" )
{
	MockLocateKeys locator;
	MockKnownPeers membership;
	MockWriteSupervisor supervisor;
	ChainWrite<MockMirrorToPeer> command(locator, membership, supervisor, false);

	// input
	readstream contents( new StringReader("contents"), KeyMetadata() );

	// output
	// nope!

	WriteInstructions params("file","v1",0,3);
	assertFalse( command.run(params, contents) );

	assertTrue( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "open(peerid,file,0)", supervisor._history.calls() );
}

TEST_CASE( "ChainWriteTest/testMultiplePackets", "[unit]" )
{
	MockLocateKeys locator;
	MockKnownPeers membership;
	MockWriteSupervisor supervisor;
	ChainWrite<MockMirrorToPeer> command(locator, membership, supervisor, false);

	// input
	readstream contents( new StringReader("contents"), KeyMetadata() );

	// output
	supervisor._writer.reset(new MockSocketWriter());

	WriteInstructions params("file","v1",0,3);
	assertTrue( command.run(params, contents) );

	assertFalse( !params.outstream );
	assertEquals( "chooseMirror(file)", MockMirrorToPeer::calls() );
	assertEquals( "open(peerid,file,0)|store(file|v1|false,8,0)", supervisor._history.calls() );

	supervisor._history.clear();

	contents = readstream( new StringReader("contents"), KeyMetadata() );
	assertTrue( command.run(params, contents) );
	assertEquals( "", MockMirrorToPeer::calls() );
	assertEquals( "store(file|v1|false,8,0)", supervisor._history.calls() );

	supervisor._history.clear();

	contents = readstream(new StringReader("contents"), KeyMetadata());
	params.isComplete = true;
	assertTrue( command.run(params, contents) );
	assertEquals( "", MockMirrorToPeer::calls() );
	assertEquals( "store(file|v1|true,8,0)", supervisor._history.calls() );
}
