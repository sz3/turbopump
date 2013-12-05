#pragma once

#include "socket/IByteStream.h"
#include <memory>
class IIpSocket;

class BufferedSocketWriter : public IByteStream
{
public:
	// TODO: should this be a weak ptr to the fd,
	// with the target peer split out into a separate data stucture?
	// for example, PeerSock{ IpAddress peer, std::weak_ptr<IIpSocket> }
	BufferedSocketWriter(const std::shared_ptr<IIpSocket>& sock);

	unsigned maxPacketLength() const;

	int read(char* buffer, unsigned length) { return 0; }
	int write(const char* buffer, unsigned length);
	int flush();

protected:
	std::string _buffer;
	std::shared_ptr<IIpSocket> _sock;
};
