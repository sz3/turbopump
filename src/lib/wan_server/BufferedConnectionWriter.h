/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IBufferedConnectionWriter.h"
#include <memory>
#include <mutex>
class IIpSocket;

// TODO: can this class be parameterized and/or swapped out (via interface)
// for the thing that handles outgoing encryption...?
// I bet it can... would make the extra buffer copy moot, since encryption needs it to happen anyway...

class BufferedConnectionWriter : public IBufferedConnectionWriter
{
public:
	BufferedConnectionWriter(const std::shared_ptr<IIpSocket>& sock, unsigned packetsize=1450);

	unsigned capacity() const;
	int write(unsigned char virtid, const char* buffer, unsigned length);
	int flush();

	void ensureDelivery_inc();
	void ensureDelivery_dec();

protected:
	void pushBytes(unsigned char virtid, const char* buff, unsigned length);
	int send(const char* buff, unsigned length);

protected:
	std::recursive_mutex _mutex;
	std::string _buffer;
	unsigned _capacity;
	std::shared_ptr<IIpSocket> _sock;
	int _blocking;
};
