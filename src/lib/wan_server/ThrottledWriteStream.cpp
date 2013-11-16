#include "ThrottledWriteStream.h"

#include "common/DataBuffer.h"
#include "socket/UdpSocket.h"
#include "wan_server/PeerConnection.h"

ThrottledWriteStream::ThrottledWriteStream(PeerConnection& conn)
	: _conn(conn)
{
}

unsigned ThrottledWriteStream::maxPacketLength() const
{
	// whatever the UDP segment size is
	return 1024;
}

int ThrottledWriteStream::read(char* buffer, unsigned length)
{
	return 0;
}

int ThrottledWriteStream::write(const char* buffer, unsigned length)
{
	return _conn.write(DataBuffer(buffer, length));
}
