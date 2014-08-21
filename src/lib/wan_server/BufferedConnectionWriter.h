/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IBufferedConnectionWriter.h"
#include <memory>
#include <mutex>
class ISocketWriter;

// TODO: can this class be parameterized and/or swapped out (via interface)
// for the thing that handles outgoing encryption...?
// I bet it can... would make the extra buffer copy moot, since encryption needs it to happen anyway...

class BufferedConnectionWriter : public IBufferedConnectionWriter
{
public:
	BufferedConnectionWriter(const std::shared_ptr<ISocketWriter>& sock, unsigned packetsize=1450);

	unsigned capacity() const;
	int write(unsigned char virtid, const char* buffer, unsigned length, bool blocking);
	bool flush(bool blocking);

protected:
	void pushBytes(unsigned char virtid, const char* buff, unsigned length);
	bool flush_internal(bool blocking);

	static unsigned findFirstTruncatedPacket(const char* buff, unsigned size);

protected:
	std::mutex _buffMutex;
	std::mutex _syncFlushMutex;
	std::string _buffer;
	unsigned _capacity;
	std::shared_ptr<ISocketWriter> _sock;
};
