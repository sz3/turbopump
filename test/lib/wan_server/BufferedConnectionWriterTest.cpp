/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "BufferedConnectionWriter.h"
#include "mock/MockIpSocket.h"
#include <memory>
#include <string>
using std::string;

namespace {
	class TestableBufferedConnectionWriter : public BufferedConnectionWriter
	{
	public:
		TestableBufferedConnectionWriter(const std::shared_ptr<IIpSocket>& sock, unsigned packetsize)
			: BufferedConnectionWriter(sock, packetsize)
		{}

		using BufferedConnectionWriter::_buffer;
	};
}

TEST_CASE( "BufferedConnectionWriterTest/testSmallBuffer.Reliable", "[unit]" )
{
	MockIpSocket* sock = new MockIpSocket;
	BufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 8);

	// a packet size of 8 will give us at most 5 data bytes. -3 bytes of overhead per write (length + virtid)

	string buff = "0123456789abc";
	assertEquals( buff.size(), writer.write(33, buff.data(), buff.size(), true) );
	assertEquals( ("send(" + string{0,6,33} + "01234)|send(" + string{0,6,33} + "56789)"), sock->_history.calls() );

	sock->_history.clear();
	buff = "FOO";
	assertEquals( buff.size(), writer.write(35, buff.data(), buff.size(), true) );
	// 33 -> previous write gets flushed == matches previous virtid
	assertEquals( ("send(" + string{0,4,33} + "abc)"), sock->_history.calls() );

	sock->_history.clear();
	writer.flush(true);
	assertEquals( ("send(" + string{0,4,35} + "FOO)"), sock->_history.calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testBigBuffer.Reliable", "[unit]" )
{
	MockIpSocket* sock = new MockIpSocket;
	BufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 1000);

	string buff = "0123456789abc";
	assertEquals( buff.size(), writer.write(37, buff.data(), buff.size(), true) );
	assertEquals( "", sock->_history.calls() );

	buff = "more bytes!!!";
	assertEquals( buff.size(), writer.write(39, buff.data(), buff.size(), true) );
	assertEquals( "", sock->_history.calls() );

	writer.flush(true);
	string header1{0, 14, 37};
	string header2{0, (char)(buff.size()+1), 39};
	assertEquals( ("send(" + header1 + "0123456789abc" + header2 + "more bytes!!!)"), sock->_history.calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testSmallBuffer.BestEffort", "[unit]" )
{
	MockIpSocket* sock = new MockIpSocket;
	BufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 8);

	// a packet size of 8 will give us at most 5 data bytes. -3 bytes of overhead per write (length + virtid)

	string buff = "0123456789abc";
	assertEquals( buff.size(), writer.write(33, buff.data(), buff.size(), false) );
	assertEquals( ("try_send(" + string{0,6,33} + "01234)|try_send(" + string{0,6,33} + "56789)"), sock->_history.calls() );

	sock->_history.clear();
	buff = "FOO";
	assertEquals( buff.size(), writer.write(35, buff.data(), buff.size(), false) );
	// 33 -> previous write gets flushed == matches previous virtid
	assertEquals( ("try_send(" + string{0,4,33} + "abc)"), sock->_history.calls() );

	sock->_history.clear();
	writer.flush(false);
	assertEquals( ("try_send(" + string{0,4,35} + "FOO)"), sock->_history.calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testBigBuffer.BestEffort", "[unit]" )
{
	MockIpSocket* sock = new MockIpSocket;
	BufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 1000);

	string buff = "0123456789abc";
	assertEquals( buff.size(), writer.write(37, buff.data(), buff.size(), false) );
	assertEquals( "", sock->_history.calls() );

	buff = "more bytes!!!";
	assertEquals( buff.size(), writer.write(39, buff.data(), buff.size(), false) );
	assertEquals( "", sock->_history.calls() );

	writer.flush(false);
	string header1{0, 14, 37};
	string header2{0, (char)(buff.size()+1), 39};
	assertEquals( ("try_send(" + header1 + "0123456789abc" + header2 + "more bytes!!!)"), sock->_history.calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testFlushFails.BestEffort", "[unit]" )
{
	MockIpSocket* sock = new MockIpSocket;
	sock->_trySendError = true;
	TestableBufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 8);

	// write succeeds w/ just 5 bytes written.
	string buff = "0123456789abc";
	assertEquals( 5, writer.write(33, buff.data(), buff.size(), false) );
	assertEquals( ("try_send(" + string{0,6,33} + "01234)"), sock->_history.calls() );
	assertEquals( (string{0,6,33} + "01234"), writer._buffer );

	// should hold on to the 5 bytes we "wrote" (but never flushed successfully)
	// next flush should try to send them.
	sock->_history.clear();
	assertFalse( writer.flush(false) );
	assertEquals( ("try_send(" + string{0,6,33} + "01234)"), sock->_history.calls() );
	assertEquals( (string{0,6,33} + "01234"), writer._buffer );

	// and if we try and write again, we shouldn't be able to write anything since the buffer is full.
	sock->_history.clear();
	buff = "56789abc";
	assertEquals( 0, writer.write(33, buff.data(), buff.size(), false) );
	// ... but we do try to flush() again
	assertEquals( ("try_send(" + string{0,6,33} + "01234)"), sock->_history.calls() );

	// now, if flush() starts to succeed...
	sock->_trySendError = false;
	sock->_history.clear();
	assertEquals( 8, writer.write(33, buff.data(), buff.size(), false) );
	assertEquals( ("try_send(" + string{0,6,33} + "01234)|try_send(" + string{0,6,33} + "56789)"), sock->_history.calls() );
	assertEquals( (string{0,4,33} + "abc"), writer._buffer );
}

TEST_CASE( "BufferedConnectionWriterTest/testFlushFails.Reliable", "[unit]" )
{
	MockIpSocket* sock = new MockIpSocket;
	sock->_trySendError = true;
	TestableBufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 10);

	// write succeeds, doesn't flush
	string buff = "01234";
	assertEquals( 5, writer.write(33, buff.data(), buff.size(), true) );
	assertEquals( "", sock->_history.calls() );
	assertEquals( (string{0,6,33} + "01234"), writer._buffer );

	// next flush should try to send them.
	sock->_history.clear();
	assertTrue( writer.flush(true) );
	assertEquals( ("send(" + string{0,6,33} + "01234)"), sock->_history.calls() );
	assertEquals( "", writer._buffer );
}
