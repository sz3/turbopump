/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "MultiplexedSocketPool.h"
#include "socket/MockSocketWriter.h"
#include "socket/mock_socket.h"
#include "socket/socket_address.h"

namespace {
	class TestableMultiplexedSocketPool : public MultiplexedSocketPool<mock_socket>
	{
	public:
		using MultiplexedSocketPool<mock_socket>::MultiplexedSocketPool;
		using MultiplexedSocketPool<mock_socket>::_connections;
	};
}

TEST_CASE( "MultiplexedSocketPoolTest/testInsert", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	assertTrue( !!pool.insert(mock_socket("1.2.3.4")) );
	assertEquals( 1, pool._connections.size() );
	assertEquals( "1.2.3.4:0", pool._connections.find(socket_address("1.2.3.4", 0))->target() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testInsert.GetWriter", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	std::shared_ptr<ISocketWriter> writer;
	assertTrue( !!pool.insert(mock_socket("1.2.3.4"), writer) );
	assertEquals( 1, pool._connections.size() );

	assertTrue( !!writer );
	assertEquals( "1.2.3.4:0", writer->target() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testInsert.AlreadyExists", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	assertTrue( !!pool.insert(mock_socket("1.2.3.4")) );
	assertEquals( 1, pool._connections.size() );

	std::shared_ptr<ISocketWriter> writer;
	assertFalse( pool.insert(mock_socket("1.2.3.4"), writer) );
	assertEquals( 1, pool._connections.size() );

	assertTrue( !!writer );
	assertEquals( "1.2.3.4:0", writer->target() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFind.NoSocket", "[unit]" )
{
	TestableMultiplexedSocketPool pool;
	assertFalse( pool.find(socket_address("1.2.3.4")) );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFind", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	mock_socket sock("1.2.3.4");
	pool._connections.insert(sock.endpoint(), std::make_shared<BufferedConnectionWriter<mock_socket>>(sock, 8));

	std::shared_ptr<ISocketWriter> writer = pool.find(socket_address("1.2.3.4"));
	assertTrue( !!writer );
	assertEquals( 5, writer->send("hello", 5) );

	std::string expected = "send(" + std::string{0,6,0} + "hello)";
	assertEquals( expected, sock.history().calls() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFindOrAdd.Find", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	mock_socket sock("1.2.3.4");
	pool._connections.insert(sock.endpoint(), std::make_shared<BufferedConnectionWriter<mock_socket>>(sock, 8));

	std::shared_ptr<ISocketWriter> writer = pool.insert(mock_socket("1.2.3.4"));
	assertTrue( !!writer );
	assertEquals( 5, writer->send("hello", 5) );

	std::string expected = "send(" + std::string{0,6,0} + "hello)";
	assertEquals( expected, sock.history().calls() );
	assertEquals( 1, pool._connections.size() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testFindOrAdd.Add", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	mock_socket sock("1.2.3.4");
	std::shared_ptr<ISocketWriter> writer = pool.insert(sock);
	assertTrue( !!writer );
	assertEquals( 5, writer->send("hello", 5) );
	assertEquals( 1, pool._connections.size() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testClose", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	mock_socket sock("1.2.3.4");
	pool._connections.insert(sock.endpoint(), std::make_shared<BufferedConnectionWriter<mock_socket>>(sock, 8));

	pool.close(sock);
	assertEquals( 0, pool._connections.size() );
	assertEquals( "close()", sock.history().calls() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testClose.NoCachedSocket", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	mock_socket sock("1.2.3.4");
	pool.close(sock);
	assertEquals( 0, pool._connections.size() );
	assertEquals( "close()", sock.history().calls() );
}

TEST_CASE( "MultiplexedSocketPoolTest/testCloseAll", "[unit]" )
{
	TestableMultiplexedSocketPool pool;

	mock_socket sock1("1.2.3.4");
	mock_socket sock2("5.6.7.8");
	mock_socket sock3("8.8.8.8");

	pool._connections.insert(sock1.endpoint(), std::make_shared<BufferedConnectionWriter<mock_socket>>(sock1, 8));
	pool._connections.insert(sock2.endpoint(), std::make_shared<BufferedConnectionWriter<mock_socket>>(sock2, 8));
	pool._connections.insert(sock3.endpoint(), std::make_shared<BufferedConnectionWriter<mock_socket>>(sock3, 8));

	pool.close_all();
	assertEquals( 0, pool._connections.size() );
	assertEquals( "close()", sock1.history().calls() );
	assertEquals( "close()", sock2.history().calls() );
	assertEquals( "close()", sock3.history().calls() );
}
