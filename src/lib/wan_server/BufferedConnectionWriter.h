#pragma once

#include "socket/IByteStream.h"
#include <memory>
class IIpSocket;

class BufferedConnectionWriter
{
public:
	// TODO: should this be a weak ptr to the fd,
	// with the target peer split out into a separate data stucture?
	// for example, PeerSock{ IpAddress peer, std::weak_ptr<IIpSocket> }
	BufferedConnectionWriter(const std::shared_ptr<IIpSocket>& sock, unsigned packetsize=1500);

	unsigned capacity() const;
	int write(unsigned char virtid, const char* buffer, unsigned length);
	int flush();

protected:
	void pushBytes(unsigned char virtid, const char* buff, unsigned length);

protected:
	std::string _buffer;
	unsigned _capacity;
	std::shared_ptr<IIpSocket> _sock;
};
