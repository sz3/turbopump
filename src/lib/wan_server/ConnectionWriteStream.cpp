#include "ConnectionWriteStream.h"

#include "IBufferedConnectionWriter.h"

// TODO: do virtual connections here?
// maybe only for buffered writes?
ConnectionWriteStream::ConnectionWriteStream(const std::shared_ptr<IBufferedConnectionWriter>& writer, unsigned char virtid)
	: _writer(writer)
	, _virtid(virtid)
{
}

unsigned ConnectionWriteStream::maxPacketLength() const
{
	return _writer->capacity();
}

int ConnectionWriteStream::write(const char* buff, unsigned length)
{
	return _writer->write(_virtid, buff, length);
}

int ConnectionWriteStream::flush()
{
	return _writer->flush();
}
