/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WriteActionSender.h"

#include "data_store/IDataStoreReader.h"
#include "membership/Peer.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockDataStore.h"
#include "mock/MockPeerTracker.h"
#include <string>
using std::string;

TEST_CASE( "WriteActionSenderTest/testDefault", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	assertTrue( client.store(Peer("dude"), "file", reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file|)|write(0,contents)|write(0,)|flush()", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testMultipleBuffers", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "0123456789abcdeABCDEturtle";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	writer->_capacity = 10;
	peers._writer.reset(writer);

	assertTrue( client.store(Peer("dude"), "file", reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file|)|write(0,0123456789)|write(0,abcdeABCDE)|write(0,turtle)|write(0,)|flush()", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testNeedsFinPacket", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers);

	// input
	MockDataStore store;
	store._store["dummy"] = "0123456789abcdeABCDE";
	IDataStoreReader::ptr reader = store.read("dummy");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	writer->_capacity = 10;
	peers._writer.reset(writer);

	assertTrue( client.store(Peer("dude"), "file", reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file|)|write(0,0123456789)|write(0,abcdeABCDE)|write(0,)|flush()", writer->_history.calls() );
}

