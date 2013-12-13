#include "unittest.h"

#include "BufferedConnectionWriter.h"
#include "mock/MockIpSocket.h"
#include <memory>
#include <string>
using std::string;

TEST_CASE( "BufferedConnectionWriterTest/testSmallBuffer", "[unit]" )
{
	MockIpSocket* sock = new MockIpSocket;
	BufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 8);

	// a packet size of 8 will give us at most 5 data bytes. -3 bytes of overhead per write (length + virtid)

	string buff = "0123456789abc";
	writer.write(33, buff.data(), buff.size());
	assertEquals( ("send(" + string{0,6,33} + "01234)|send(" + string{0,6,33} + "56789)"), sock->_history.calls() );

	sock->_history.clear();
	buff = "FOO";
	writer.write(35, buff.data(), buff.size());
	// 33 -> previous write gets flushed == matches previous virtid
	assertEquals( ("send(" + string{0,4,33} + "abc)"), sock->_history.calls() );

	sock->_history.clear();
	writer.flush();
	assertEquals( ("send(" + string{0,4,35} + "FOO)"), sock->_history.calls() );
}

TEST_CASE( "BufferedConnectionWriterTest/testBigBuffer", "[unit]" )
{
	MockIpSocket* sock = new MockIpSocket;
	BufferedConnectionWriter writer(std::shared_ptr<IIpSocket>(sock), 1000);

	string buff = "0123456789abc";
	writer.write(37, buff.data(), buff.size());
	assertEquals( "", sock->_history.calls() );

	sock->_history.clear();
	buff = "more bytes!!!";
	writer.write(39, buff.data(), buff.size());
	assertEquals( "", sock->_history.calls() );

	writer.flush();
	string header1{0, 14, 37};
	string header2{0, (char)(buff.size()+1), 39};
	assertEquals( ("send(" + header1 + "0123456789abc" + header2 + "more bytes!!!)"), sock->_history.calls() );
}

