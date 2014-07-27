/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WriteActionSender.h"

#include "actions/WriteParams.h"
#include "data_store/IDataStoreReader.h"
#include "membership/Peer.h"
#include "wan_server/ConnectionWriteStream.h"
#include "mock/MockBufferedConnectionWriter.h"
#include "mock/MockDataStore.h"
#include "mock/MockPeerTracker.h"
#include <string>
using std::shared_ptr;
using std::string;

namespace {
	class TestableConnectionWriteStream : public ConnectionWriteStream
	{
	public:
		using ConnectionWriteStream::_writer;
		using ConnectionWriteStream::_blocking;
	};
}

TEST_CASE( "WriteActionSenderTest/testOpenAndStore", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	shared_ptr<ConnectionWriteStream> conn = client.open(Peer("dude"), params);
	assertFalse( !conn );
	assertFalse( ((TestableConnectionWriteStream&)*conn)._blocking );
	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0|,false)", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,false)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	reader = store.read("dummy", "version");
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,false)|write(0,,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testOpenAndStore.LastPacketEmpty", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	shared_ptr<ConnectionWriteStream> conn = client.open(Peer("dude"), params);
	assertFalse( !conn );
	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0|,false)", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,false)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testOpenAndStore.Blocking", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, true);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	shared_ptr<ConnectionWriteStream> conn = client.open(Peer("dude"), params);
	assertFalse( !conn );
	assertTrue( ((TestableConnectionWriteStream&)*conn)._blocking );
	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0|,true)", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,true)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	reader = store.read("dummy", "version");
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,true)|write(0,,true)|flush(true)", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testDefault", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0|,false)|write(0,contents,false)", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testEnsureDelivery", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, true);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0|,true)|write(0,contents,true)", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testOpen", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, true);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0|,true)|write(0,contents,true)", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testWithSource", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	params.source = "dude";
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0 source=dude|,false)|write(0,contents,false)", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testMultipleBuffers", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "0123456789abcdeABCDEturtle";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	writer->_capacity = 10;
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0|,false)|write(0,0123456789,false)|write(0,abcdeABCDE,false)|write(0,turtle,false)", writer->_history.calls() );
}

TEST_CASE( "WriteActionSenderTest/testNeedsFinPacket", "[unit]" )
{
	MockPeerTracker peers;
	WriteActionSender client(peers, false);

	// input
	MockDataStore store;
	store._store["dummy"] = "0123456789abcdeABCDE";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockBufferedConnectionWriter* writer = new MockBufferedConnectionWriter();
	writer->_capacity = 10;
	peers._writer.reset(writer);

	WriteParams params("file",2,3,"v1",0);
	params.isComplete = true;
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", peers._history.calls() );
	assertEquals( "write(0,write|name=file i=2 n=3 v=v1 offset=0|,false)|write(0,0123456789,false)|write(0,abcdeABCDE,false)|write(0,,false)|flush(false)", writer->_history.calls() );
}

