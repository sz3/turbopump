/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "BufferedConnectionWriter.h"
#include "socket/mock_socket.h"
#include <memory>
#include <string>
using std::string;

namespace {
	class TestableBufferedConnectionWriter : public BufferedConnectionWriter<mock_socket>
	{
	public:
		using BufferedConnectionWriter::BufferedConnectionWriter;

		using BufferedConnectionWriter::findFirstTruncatedPacket;
		using BufferedConnectionWriter::_buffer;
		using BufferedConnectionWriter::_sock;
	};
}

TEST_CASE( "BufferedConnectionWriterTest/testSmallBuffer.Reliable", "[unit]" )
{
	mock_socket sock("localhost");
	BufferedConnectionWriter<mock_socket> writer(sock, 8);

	// a packet size of 8 will give us at most 5 data bytes. -3 bytes of overhead per write (length + virtid)

	string buff = "0123456789abc";
	assertEquals( buff.size(), writer.write(33, buff.data(), buff.size(), true) );
	assertEquals( ("send(" + string{0,6,33} + "01234)|send(" + string{0,6,33} + "56789)"), sock.history().calls() );

	sock.history().clear();
	buff = "FOO";
	assertEquals( buff.size(), writer.write(35, buff.data(), buff.size(), true) );
	// 33 -> previous write gets flushed == matches previous virtid
	assertEquals( ("send(" + string{0,4,33} + "abc)"), sock.history().calls() );

	sock.history().clear();
	writer.flush(true);
	assertEquals( ("send(" + string{0,4,35} + "FOO)"), sock.history().calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testBigBuffer.Reliable", "[unit]" )
{
	mock_socket sock("localhost");
	BufferedConnectionWriter<mock_socket> writer(sock, 1000);

	string buff = "0123456789abc";
	assertEquals( buff.size(), writer.write(37, buff.data(), buff.size(), true) );
	assertEquals( "", sock.history().calls() );

	buff = "more bytes!!!";
	assertEquals( buff.size(), writer.write(39, buff.data(), buff.size(), true) );
	assertEquals( "", sock.history().calls() );

	writer.flush(true);
	string header1{0, 14, 37};
	string header2{0, (char)(buff.size()+1), 39};
	assertEquals( ("send(" + header1 + "0123456789abc" + header2 + "more bytes!!!)"), sock.history().calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testSmallBuffer.BestEffort", "[unit]" )
{
	mock_socket sock("localhost");
	BufferedConnectionWriter<mock_socket> writer(sock, 8);

	// a packet size of 8 will give us at most 5 data bytes. -3 bytes of overhead per write (length + virtid)

	string buff = "0123456789abc";
	assertEquals( buff.size(), writer.write(33, buff.data(), buff.size(), false) );
	assertEquals( ("try_send(" + string{0,6,33} + "01234)|try_send(" + string{0,6,33} + "56789)"), sock.history().calls() );

	sock.history().clear();
	buff = "FOO";
	assertEquals( buff.size(), writer.write(35, buff.data(), buff.size(), false) );
	// 33 -> previous write gets flushed == matches previous virtid
	assertEquals( ("try_send(" + string{0,4,33} + "abc)"), sock.history().calls() );

	sock.history().clear();
	writer.flush(false);
	assertEquals( ("try_send(" + string{0,4,35} + "FOO)"), sock.history().calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testBigBuffer.BestEffort", "[unit]" )
{
	mock_socket sock("localhost");
	BufferedConnectionWriter<mock_socket> writer(sock, 1000);

	string buff = "0123456789abc";
	assertEquals( buff.size(), writer.write(37, buff.data(), buff.size(), false) );
	assertEquals( "", sock.history().calls() );

	buff = "more bytes!!!";
	assertEquals( buff.size(), writer.write(39, buff.data(), buff.size(), false) );
	assertEquals( "", sock.history().calls() );

	writer.flush(false);
	string header1{0, 14, 37};
	string header2{0, (char)(buff.size()+1), 39};
	assertEquals( ("try_send(" + header1 + "0123456789abc" + header2 + "more bytes!!!)"), sock.history().calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testFlushFails.BestEffort", "[unit]" )
{
	mock_socket sock("localhost");
	sock._trySendError = -1;
	TestableBufferedConnectionWriter writer(sock, 8);

	// write succeeds w/ just 5 bytes written.
	string buff = "0123456789abc";
	assertEquals( 5, writer.write(33, buff.data(), buff.size(), false) );
	assertEquals( ("try_send(" + string{0,6,33} + "01234)"), sock.history().calls() );
	assertEquals( (string{0,6,33} + "01234"), writer._buffer );

	// should hold on to the 5 bytes we "wrote" (but never flushed successfully)
	// next flush should try to send them.
	sock.history().clear();
	assertFalse( writer.flush(false) );
	assertEquals( ("try_send(" + string{0,6,33} + "01234)"), sock.history().calls() );
	assertEquals( (string{0,6,33} + "01234"), writer._buffer );

	// and if we try and write again, we shouldn't be able to write anything since the buffer is full.
	sock.history().clear();
	buff = "56789abc";
	assertEquals( 0, writer.write(33, buff.data(), buff.size(), false) );
	// ... but we do try to flush() again
	assertEquals( ("try_send(" + string{0,6,33} + "01234)"), sock.history().calls() );

	// now, if flush() starts to succeed...
	writer._sock._trySendError = false;
	sock.history().clear();
	assertEquals( 8, writer.write(33, buff.data(), buff.size(), false) );
	assertEquals( ("try_send(" + string{0,6,33} + "01234)|try_send(" + string{0,6,33} + "56789)"), sock.history().calls() );
	assertEquals( (string{0,4,33} + "abc"), writer._buffer );
}

TEST_CASE( "BufferedConnectionWriterTest/testFlushFails.BestEffort.PartialPacket", "[unit]" )
{
	mock_socket sock("localhost");
	sock._trySendError = true;
	sock._trySendErrorBytes = 20; // claim to have sent 20 bytes on failure
	TestableBufferedConnectionWriter writer(sock, 50);

	// write succeeds, but only flushes twice
	string buff = "0123456789";
	assertEquals( 10, writer.write(33, buff.data(), buff.size(), false) );

	buff = "abcdef";
	assertEquals( 6, writer.write(35, buff.data(), buff.size(), false) );
	assertEquals( "", sock.history().calls() );
	assertEquals( (string{0,11,33} + "0123456789" + (string{0,7,35} + "abcdef")), writer._buffer );

	// flush partially fails
	sock.history().clear();
	assertFalse( writer.flush(false) );
	assertEquals( ("try_send(" + string{0,11,33} + "0123456789" + string{0,7,35} + "abcdef)"), sock.history().calls() );

	// hold onto the *entire* failed packet. We sent 20 bytes successfully, but we needed to send 22...
	assertEquals( (string{0,7,35} + "abcdef"), writer._buffer );
}

TEST_CASE( "BufferedConnectionWriterTest/testFlushFails.Reliable", "[unit]" )
{
	mock_socket sock("localhost");
	sock._trySendError = true;
	TestableBufferedConnectionWriter writer(sock, 10);

	// write succeeds, doesn't flush
	string buff = "01234";
	assertEquals( 5, writer.write(33, buff.data(), buff.size(), true) );
	assertEquals( "", sock.history().calls() );
	assertEquals( (string{0,6,33} + "01234"), writer._buffer );

	// next flush should try to send them.
	sock.history().clear();
	assertTrue( writer.flush(true) );
	assertEquals( ("send(" + string{0,6,33} + "01234)"), sock.history().calls() );
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
