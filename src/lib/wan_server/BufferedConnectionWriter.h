#pragma once

#include "IBufferedConnectionWriter.h"
#include "socket/IByteStream.h"
#include <memory>
#include <mutex>
class IIpSocket;

// TODO: scheduler thread!
// set up nagle-style flush at ~200 ms.

// TODO: can this class be parameterized and/or swapped out (via interface)
// for the thing that handles outgoing encryption...?
// I bet it can... would make the extra buffer copy moot, since encryption needs it to happen anyway...

class BufferedConnectionWriter : public IBufferedConnectionWriter
{
public:
	// TODO: should this be a weak ptr to the fd,
	// with the target peer split out into a separate data stucture?
	// for example, PeerSock{ IpAddress peer, std::weak_ptr<IIpSocket> }
	BufferedConnectionWriter(const std::shared_ptr<IIpSocket>& sock, unsigned packetsize=1450);

	unsigned capacity() const;
	int write(unsigned char virtid, const char* buffer, unsigned length);
	int flush();

protected:
	void pushBytes(unsigned char virtid, const char* buff, unsigned length);

protected:
	std::recursive_mutex _mutex;
	std::string _buffer;
	unsigned _capacity;
	std::shared_ptr<IIpSocket> _sock;
};
