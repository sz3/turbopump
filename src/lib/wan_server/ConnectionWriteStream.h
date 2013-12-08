#pragma once

#include "socket/IByteStream.h"
#include <memory>
class IBufferedConnectionWriter;

class ConnectionWriteStream : public IByteStream
{
public:
	ConnectionWriteStream(const std::shared_ptr<IBufferedConnectionWriter>& writer, unsigned char virtid);

	unsigned maxPacketLength() const;

	int read(char* buffer, unsigned length) { return 0; }
	int write(const char* buffer, unsigned length);
	int flush();

protected:
	std::shared_ptr<IBufferedConnectionWriter> _writer;
	unsigned char _virtid;
};
