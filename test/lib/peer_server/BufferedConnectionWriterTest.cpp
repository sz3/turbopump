/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "BufferedConnectionWriter.h"
#include "socket/MockSocketWriter.h"
#include <memory>
#include <string>
using std::string;

namespace {
	class TestableBufferedConnectionWriter : public BufferedConnectionWriter
	{
	public:
		TestableBufferedConnectionWriter(const std::shared_ptr<ISocketWriter>& sock, unsigned packetsize)
			: BufferedConnectionWriter(sock, packetsize)
		{}

		using BufferedConnectionWriter::findFirstTruncatedPacket;
		using BufferedConnectionWriter::_buffer;
	};
}

TEST_CASE( "BufferedConnectionWriterTest/testSmallBuffer.Reliable", "[unit]" )
{
	MockSocketWriter* sock = new MockSocketWriter;
	BufferedConnectionWriter writer(std::shared_ptr<ISocketWriter>(sock), 8);

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
	MockSocketWriter* sock = new MockSocketWriter;
	BufferedConnectionWriter writer(std::shared_ptr<ISocketWriter>(sock), 1000);

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
	MockSocketWriter* sock = new MockSocketWriter;
	BufferedConnectionWriter writer(std::shared_ptr<ISocketWriter>(sock), 8);

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
	MockSocketWriter* sock = new MockSocketWriter;
	BufferedConnectionWriter writer(std::shared_ptr<ISocketWriter>(sock), 1000);

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
	MockSocketWriter* sock = new MockSocketWriter;
	sock->_trySendError = true;
	TestableBufferedConnectionWriter writer(std::shared_ptr<ISocketWriter>(sock), 8);

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

TEST_CASE( "BufferedConnectionWriterTest/testFlushFails.BestEffort.PartialPacket", "[unit]" )
{
	MockSocketWriter* sock = new MockSocketWriter;
	sock->_trySendError = true;
	sock->_trySendErrorBytes = 20; // claim to have sent 20 bytes on failure
	TestableBufferedConnectionWriter writer(std::shared_ptr<ISocketWriter>(sock), 50);

	// write succeeds, but only flushes twice
	string buff = "0123456789";
	assertEquals( 10, writer.write(33, buff.data(), buff.size(), false) );

	buff = "abcdef";
	assertEquals( 6, writer.write(35, buff.data(), buff.size(), false) );
	assertEquals( "", sock->_history.calls() );
	assertEquals( (string{0,11,33} + "0123456789" + (string{0,7,35} + "abcdef")), writer._buffer );

	// flush partially fails
	sock->_history.clear();
	assertFalse( writer.flush(false) );
	assertEquals( ("try_send(" + string{0,11,33} + "0123456789" + string{0,7,35} + "abcdef)"), sock->_history.calls() );

	// hold onto the *entire* failed packet. We sent 20 bytes successfully, but we needed to send 22...
	assertEquals( (string{0,7,35} + "abcdef"), writer._buffer );
}

TEST_CASE( "BufferedConnectionWriterTest/testFlushFails.Reliable", "[unit]" )
{
	MockSocketWriter* sock = new MockSocketWriter;
	sock->_trySendError = true;
	TestableBufferedConnectionWriter writer(std::shared_ptr<ISocketWriter>(sock), 10);

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

TEST_CASE( "BufferedConnectionWriterTest/testFindFirstTruncatedPacket", "[unit]" )
{
	string buff = (string{0,6,33} + "01234") + (string{0,6,33} + "abcde");
	unsigned index = TestableBufferedConnectionWriter::findFirstTruncatedPacket(buff.data(), 15);
	assertEquals( 8, index );
	assertEquals( (string{0,6,33} + "abcde"), buff.substr(index) );

	index = TestableBufferedConnectionWriter::findFirstTruncatedPacket(buff.data(), 9);
	assertEquals( 8, index );
	assertEquals( (string{0,6,33} + "abcde"), buff.substr(index) );

	index = TestableBufferedConnectionWriter::findFirstTruncatedPacket(buff.data(), 8);
	assertEquals( 8, index );
	assertEquals( (string{0,6,33} + "abcde"), buff.substr(index) );

	index = TestableBufferedConnectionWriter::findFirstTruncatedPacket(buff.data(), 7);
	assertEquals( 0, index );
	assertEquals( buff, buff.substr(index) );

	index = TestableBufferedConnectionWriter::findFirstTruncatedPacket(buff.data(), 1);
	assertEquals( 0, index );
	assertEquals( buff, buff.substr(index) );

	index = TestableBufferedConnectionWriter::findFirstTruncatedPacket(buff.data(), buff.size());
	assertEquals( buff.size(), index );
	assertEquals( "", buff.substr(index) );
}
