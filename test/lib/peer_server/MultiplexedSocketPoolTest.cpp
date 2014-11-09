/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MultiplexedSocketPool.h"
#include "mock/MockMembership.h"
#include "socket/MockSocketWriter.h"
#include "socket/mock_socket.h"

namespace {
	class TestableMultiplexedSocketPool : public MultiplexedSocketPool<mock_socket>
	{
	public:
		using MultiplexedSocketPool<mock_socket>::MultiplexedSocketPool;
		using MultiplexedSocketPool<mock_socket>::_connections;
	};
}

TEST_CASE( "MultiplexedSocketPoolTest/testAdd", "[unit]" )
{
	MockMembership membership;
	membership._ips["1.2.3.4:0"] = std::shared_ptr<Peer>(new Peer("one"));
	TestableMultiplexedSocketPool pool(membership);

	assertTrue( pool.add(mock_socket("1.2.3.4")) );
	assertEquals( "lookupIp(1.2.3.4:0)", membership._history.calls() );
	assertEquals( 1, pool._connections.size() );
	assertEquals( "1.2.3.4:0", pool._connections["1.2.3.4:0"]->target() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testAdd.BadPeer", "[unit]" )
{
	MockMembership membership;
	TestableMultiplexedSocketPool pool(membership);

	assertFalse( pool.add(mock_socket("1.2.3.4")) );
	assertTrue( pool._connections.empty() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testAdd.GetWriter", "[unit]" )
{
	MockMembership membership;
	membership._ips["1.2.3.4:0"] = std::shared_ptr<Peer>(new Peer("one"));
	TestableMultiplexedSocketPool pool(membership);

	std::shared_ptr<ISocketWriter> writer;
	assertTrue( pool.add(mock_socket("1.2.3.4"), writer) );
	assertEquals( "lookupIp(1.2.3.4:0)", membership._history.calls() );
	assertEquals( 1, pool._connections.size() );

	assertTrue( !!writer );
	assertEquals( "1.2.3.4:0", writer->target() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testAdd.AlreadyExists", "[unit]" )
{
	MockMembership membership;
	membership._ips["1.2.3.4:0"] = std::shared_ptr<Peer>(new Peer("one"));
	TestableMultiplexedSocketPool pool(membership);

	assertTrue( pool.add(mock_socket("1.2.3.4")) );
	assertEquals( 1, pool._connections.size() );

	membership._history.clear();

	std::shared_ptr<ISocketWriter> writer;
	assertFalse( pool.add(mock_socket("1.2.3.4"), writer) );
	assertEquals( "lookupIp(1.2.3.4:0)", membership._history.calls() );
	assertEquals( 1, pool._connections.size() );

	assertTrue( !!writer );
	assertEquals( "1.2.3.4:0", writer->target() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFind.BadPeer", "[unit]" )
{
	MockMembership membership;
	TestableMultiplexedSocketPool pool(membership);
	assertFalse( pool.find(socket_address("1.2.3.4")) );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFind.NoSocket", "[unit]" )
{
	MockMembership membership;
	membership._ips["1.2.3.4:0"] = std::shared_ptr<Peer>(new Peer("one"));
	TestableMultiplexedSocketPool pool(membership);
	assertFalse( pool.find(socket_address("1.2.3.4")) );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFind", "[unit]" )
{
	MockMembership membership;
	membership._ips["1.2.3.4:0"] = std::shared_ptr<Peer>(new Peer("one"));
	TestableMultiplexedSocketPool pool(membership);

	mock_socket sock("1.2.3.4");
	pool._connections["1.2.3.4:0"] = std::make_shared<BufferedConnectionWriter<mock_socket>>(sock, 8);

	std::shared_ptr<ISocketWriter> writer = pool.find(socket_address("1.2.3.4"));
	assertTrue( !!writer );
	assertEquals( 5, writer->send("hello", 5) );

	std::string expected = "send(" + std::string{0,6,0} + "hello)";
	assertEquals( expected, sock.history().calls() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFindOrAdd.Find", "[unit]" )
{
	MockMembership membership;
	membership._ips["1.2.3.4:0"] = std::shared_ptr<Peer>(new Peer("one"));
	TestableMultiplexedSocketPool pool(membership);

	mock_socket sock("1.2.3.4");
	pool._connections["1.2.3.4:0"] = std::make_shared<BufferedConnectionWriter<mock_socket>>(sock, 8);

	std::shared_ptr<ISocketWriter> writer = pool.find_or_add(mock_socket("1.2.3.4"));
	assertTrue( !!writer );
	assertEquals( 5, writer->send("hello", 5) );

	std::string expected = "send(" + std::string{0,6,0} + "hello)";
	assertEquals( expected, sock.history().calls() );

	assertEquals( "", membership._history.calls() );
	assertEquals( 1, pool._connections.size() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFindOrAdd.Add", "[unit]" )
{
	MockMembership membership;
	membership._ips["1.2.3.4:0"] = std::shared_ptr<Peer>(new Peer("one"));
	TestableMultiplexedSocketPool pool(membership);

	mock_socket sock("1.2.3.4");
	std::shared_ptr<ISocketWriter> writer = pool.find_or_add(sock);
	assertTrue( !!writer );
	assertEquals( 5, writer->send("hello", 5) );

	assertEquals( "lookupIp(1.2.3.4:0)", membership._history.calls() );
	assertEquals( 1, pool._connections.size() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testClose", "[unit]" )
{
	MockMembership membership;
	TestableMultiplexedSocketPool pool(membership);

	mock_socket sock("1.2.3.4");
	pool._connections["1.2.3.4:0"] = std::make_shared<BufferedConnectionWriter<mock_socket>>(sock, 8);

	pool.close(sock);
	assertEquals( 0, pool._connections.size() );
	assertEquals( "close()", sock.history().calls() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testClose.NoCachedSocket", "[unit]" )
{
	MockMembership membership;
	TestableMultiplexedSocketPool pool(membership);

	mock_socket sock("1.2.3.4");
	pool.close(sock);
	assertEquals( 0, pool._connections.size() );
	assertEquals( "close()", sock.history().calls() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testCloseAll", "[unit]" )
{
	MockMembership membership;
	TestableMultiplexedSocketPool pool(membership);

	mock_socket sock1("1.2.3.4");
	mock_socket sock2("5.6.7.8");
	mock_socket sock3("8.8.8.8");

	pool._connections["1.2.3.4:0"] = std::make_shared<BufferedConnectionWriter<mock_socket>>(sock1, 8);
	pool._connections["5.6.7.8:0"] = std::make_shared<BufferedConnectionWriter<mock_socket>>(sock2, 8);
	pool._connections["8.8.8.8:0"] = std::make_shared<BufferedConnectionWriter<mock_socket>>(sock3, 8);

	pool.close_all();
	assertEquals( 0, pool._connections.size() );
	assertEquals( "close()", sock1.history().calls() );
	assertEquals( "close()", sock2.history().calls() );
	assertEquals( "close()", sock3.history().calls() );
}
