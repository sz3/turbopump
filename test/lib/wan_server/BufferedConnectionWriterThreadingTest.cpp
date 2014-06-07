/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "BufferedConnectionWriter.h"
#include "mock/MockIpSocket.h"
#include "time/WaitFor.h"
#include <string>
#include <thread>
#include <vector>
using std::string;

namespace {
	class SpecialMockSocket : public IIpSocket
	{
	public:
		SpecialMockSocket()
			: _blockingWriters(0)
			, _blockingCollisions(0)
			, _asyncWriters(0)
			, _asyncCollisions(0)
			, _writeBytes(0)
		{}

		IpAddress getTarget() const { return IpAddress(); }
		std::string destination() const { return ""; }

		int try_send(const char* buffer, unsigned size) const
		{
			if (++_asyncWriters > 1)
				++_asyncCollisions;
			_writeBytes += size;
			--_asyncWriters;
			return size;
		}

		int send(const char* buffer, unsigned size) const
		{
			if (++_blockingWriters > 1)
				++_blockingCollisions;
			_writeBytes += size;
			--_blockingWriters;
			return size;
		}

		int recv(std::string& buffer) { return 0; }

	public:
		mutable int _blockingWriters;
		mutable int _blockingCollisions;
		mutable int _asyncWriters;
		mutable int _asyncCollisions;
		mutable unsigned _writeBytes;

	};
}

TEST_CASE( "BufferedConnectionWriterThreadingTest/testMultiThreading", "[unit]" )
{
	SpecialMockSocket* sock = new SpecialMockSocket;
	BufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 15);

	string buff = "0123456789";
	auto blocking = [&] () { writer.write(33, buff.data(), buff.size(), true); };
	auto async = [&] () { writer.write(33, buff.data(), buff.size(), false); };

	std::vector<std::thread> threads;
	for (int i = 0; i < 30; ++i)
	{
		threads.push_back( std::thread(blocking) );
		threads.push_back( std::thread(async) );
	}

	for (auto it = threads.begin(); it != threads.end(); ++it)
		(*it).join();
	writer.flush(true);

	assertEquals( 0, sock->_blockingWriters );
	assertEquals( 0, sock->_blockingCollisions );
	assertEquals( 0, sock->_asyncWriters );
	assertEquals( 0, sock->_asyncCollisions );
	assertEquals( (30*13*2), sock->_writeBytes );
}

