/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "WriteSupervisor.h"

#include "api/WriteInstructions.h"
#include "common/KeyMetadata.h"
#include "membership/Peer.h"
#include "mock/MockPartialTransfers.h"
#include "mock/MockRequestPacker.h"
#include "mock/MockStore.h"
#include "peer_server/ConnectionWriteStream.h"
#include "storage/readstream.h"
#include "storage/StringReader.h"

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

	Peer mockPeer(std::string ip)
	{
		Peer peer("foo");
		peer.ips.push_back(ip);
		return peer;
	}
}

TEST_CASE( "WriteSupervisorTest/testOpenAndStore", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	WriteSupervisor client(packer, transfers, server, store);

	// input
	readstream reader(new StringReader("contents"), KeyMetadata());

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	shared_ptr<ConnectionWriteStream> conn = client.open(mockPeer("1.2.3.4:80"), params, false);
	assertFalse( !conn );
	assertFalse( ((TestableConnectionWriteStream&)*conn)._blocking );
	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "try_send({100 copies=3 mirror=2 name=file offset=0 source= version=v1})", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "try_send(contents)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	reader = readstream(new StringReader("contents"), KeyMetadata());
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "try_send(contents)|try_send()|flush(false)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testOpenAndStore.LastPacketEmpty", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	WriteSupervisor client(packer, transfers, server, store);

	// input
	readstream reader(new StringReader("contents"), KeyMetadata());

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	shared_ptr<ConnectionWriteStream> conn = client.open(mockPeer("1.2.3.4:80"), params, false);
	assertFalse( !conn );
	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "try_send({100 copies=3 mirror=2 name=file offset=0 source= version=v1})", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "try_send(contents)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "try_send()|flush(false)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testOpenAndStore.Blocking", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	WriteSupervisor client(packer, transfers, server, store);

	// input
	readstream reader(new StringReader("contents"), KeyMetadata());

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	shared_ptr<ConnectionWriteStream> conn = client.open(mockPeer("1.2.3.4:80"), params, true);
	assertFalse( !conn );
	assertTrue( ((TestableConnectionWriteStream&)*conn)._blocking );
	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "send({100 copies=3 mirror=2 name=file offset=0 source= version=v1})", writer->_history.calls() );

	writer->_history.clear();
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "send(contents)", writer->_history.calls() );

	writer->_history.clear();
	params.isComplete = true;
	reader = readstream(new StringReader("contents"), KeyMetadata());
	assertTrue( client.store(*conn, params, reader) );
	assertEquals( "send(contents)|send()|flush(true)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testDefault", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	WriteSupervisor client(packer, transfers, server, store);

	// input
	readstream reader(new StringReader("contents"), KeyMetadata());

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	assertTrue( client.store(mockPeer("1.2.3.4:80"), params, reader) );

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "send({100 copies=3 mirror=2 name=file offset=0 source= version=v1})|send(contents)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testWithSource", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	WriteSupervisor client(packer, transfers, server, store);

	// input
	readstream reader(new StringReader("contents"), KeyMetadata());

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	params.source = "dude";
	assertTrue( client.store(mockPeer("1.2.3.4:80"), params, reader) );

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "send({100 copies=3 mirror=2 name=file offset=0 source=dude version=v1})"
				  "|send(contents)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testMultipleBuffers", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	WriteSupervisor client(packer, transfers, server, store);

	// input
	readstream reader(new StringReader("0123456789abcdeABCDEturtle", 10), KeyMetadata());

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	assertTrue( client.store(mockPeer("1.2.3.4:80"), params, reader) );

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "send({100 copies=3 mirror=2 name=file offset=0 source= version=v1})"
				  "|send(0123456789)|send(abcdeABCDE)|send(turtle)", writer->_history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testNeedsFinPacket", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	WriteSupervisor client(packer, transfers, server, store);

	// input
	readstream reader(new StringReader("0123456789abcdeABCDE", 10), KeyMetadata());

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	params.isComplete = true;
	assertTrue( client.store(mockPeer("1.2.3.4:80"), params, reader) );

	assertEquals( "getWriter(1.2.3.4:80)", server._history.calls() );
	assertEquals( "package(100)", packer._history.calls() );
	assertEquals( "send({100 copies=3 mirror=2 name=file offset=0 source= version=v1})"
				  "|send(0123456789)|send(abcdeABCDE)|send()|flush(true)", writer->_history.calls() );
}

namespace {
	class TestableWriteSupervisor : public WriteSupervisor
	{
	public:
		using WriteSupervisor::WriteSupervisor;
		using WriteSupervisor::resume;
	};

	class BadReader : public IReader
	{
	public:
		bool good() const
		{
			return true;
		}

		unsigned long long size() const
		{
			return 10;
		}

		bool setPosition(unsigned long long offset)
		{
			_history.call("setPosition", offset);
			return 0;
		}

		int stream(IByteStream& sink)
		{
			_history.call("stream");
			_bytes += 5;
			if (_bytes >= 10)
				return -1; // one "good" write, then... kaboom!
			return _bytes;
		}

	public:
		CallHistory _history;
		int _bytes = 0;
	};
}

TEST_CASE( "WriteSupervisorTest/testRewrite", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	store._reads["file"] = "contents";
	TestableWriteSupervisor client(packer, transfers, server, store);

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	params.isComplete = true;
	params.outstream.reset(new ConnectionWriteStream(server._sock, false));
	assertTrue( client.resume(params) );

	assertEquals( "try_send(contents)|try_send()|flush(false)", writer->_history.calls() );
	assertEquals( "", server._history.calls() );
	assertEquals( "", packer._history.calls() );
}

TEST_CASE( "WriteSupervisorTest/testWrite.ScheduleRewrite", "[unit]" )
{
	MockRequestPacker packer;
	MockPartialTransfers transfers;
	MockSocketServer server;
	MockStore store;
	TestableWriteSupervisor client(packer, transfers, server, store);

	// input
	BadReader* meReader = new BadReader();
	readstream reader(meReader, KeyMetadata());

	// output
	MockSocketWriter* writer = new MockSocketWriter();
	server._sock.reset(writer);

	WriteInstructions params("file","v1",2,3);
	params.isComplete = true;
	params.outstream.reset(new ConnectionWriteStream(server._sock, false));
	assertFalse( client.store(*params.outstream, params, reader) );

	assertEquals( "target()", writer->_history.calls() );
	assertEquals( "stream()|stream()", meReader->_history.calls() );
	assertEquals( "add()", transfers._history.calls() );

	writer->_history.clear();
	meReader->_history.clear();
	transfers._history.clear();

	// fire the transfers callback. Should be a WriteInstructions::resume()
	store._reads["file"] = "contents";
	transfers._capturedFun();

	assertEquals( "try_send(contents)|try_send()|flush(false)", writer->_history.calls() );
	assertEquals( "", meReader->_history.calls() );
	assertEquals( "", transfers._history.calls() );
	assertEquals( "", server._history.calls() );
	assertEquals( "", packer._history.calls() );
}

