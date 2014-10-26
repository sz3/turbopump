/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WriteSupervisor.h"

#include "api/WriteInstructions.h"
#include "data_store/IDataStoreReader.h"
#include "membership/Peer.h"
#include "mock/MockDataStore.h"
#include "mock/MockRequestPacker.h"
#include "peer_server/ConnectionWriteStream.h"
#include "socket/MockSocketServer.h"
#include "socket/MockSocketWriter.h"
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

TEST_CASE( "WriteSupervisorTest/testOpenAndStore", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	WriteSupervisor client(packer, server);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	shared_ptr<ConnectionWriteStream> conn = client.open(Peer("dude"), params, false);
	assertFalse( !conn );
	assertFalse( ((TestableConnectionWriteStream&)*conn)._blocking );
	assertEquals( "getWriter(dude)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "write(0,{100 copies=3 mirror=2 name=file offset=0 source= version=v1},false)", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,false)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	reader = store.read("dummy", "version");
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,false)|write(0,,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testOpenAndStore.LastPacketEmpty", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	WriteSupervisor client(packer, server);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	shared_ptr<ConnectionWriteStream> conn = client.open(Peer("dude"), params, false);
	assertFalse( !conn );
	assertEquals( "getWriter(dude)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "write(0,{100 copies=3 mirror=2 name=file offset=0 source= version=v1},false)", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,false)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,,false)|flush(false)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testOpenAndStore.Blocking", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	WriteSupervisor client(packer, server);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	shared_ptr<ConnectionWriteStream> conn = client.open(Peer("dude"), params, true);
	assertFalse( !conn );
	assertTrue( ((TestableConnectionWriteStream&)*conn)._blocking );
	assertEquals( "getWriter(dude)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "write(0,{100 copies=3 mirror=2 name=file offset=0 source= version=v1},true)", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,true)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	reader = store.read("dummy", "version");
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "write(0,contents,true)|write(0,,true)|flush(true)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testDefault", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	WriteSupervisor client(packer, server);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "write(0,{100 copies=3 mirror=2 name=file offset=0 source= version=v1},true)|write(0,contents,true)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testWithSource", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	WriteSupervisor client(packer, server);

	// input
	MockDataStore store;
	store._store["dummy"] = "contents";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	params.source = "dude";
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "write(0,{100 copies=3 mirror=2 name=file offset=0 source=dude version=v1},true)"
				  "|write(0,contents,true)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testMultipleBuffers", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	WriteSupervisor client(packer, server);

	// input
	MockDataStore store;
	store._store["dummy"] = "0123456789abcdeABCDEturtle";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	writer->_capacity = 10;
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "write(0,{100 copies=3 mirror=2 name=file offset=0 source= version=v1},true)"
				  "|write(0,0123456789,true)|write(0,abcdeABCDE,true)|write(0,turtle,true)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testNeedsFinPacket", "[unit]" )
{
	MockRequestPacker packer;
	MockSocketServer server;
	WriteSupervisor client(packer, server);

	// input
	MockDataStore store;
	store._store["dummy"] = "0123456789abcdeABCDE";
	IDataStoreReader::ptr reader = store.read("dummy", "version");

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	writer->_capacity = 10;
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	params.isComplete = true;
	assertTrue( client.store(Peer("dude"), params, reader) );

	assertEquals( "getWriter(dude)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "write(0,{100 copies=3 mirror=2 name=file offset=0 source= version=v1},true)"
				  "|write(0,0123456789,true)|write(0,abcdeABCDE,true)|write(0,,true)|flush(true)", writer->_history.calls() );
}

