/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ConnectionWriteStream.h"

#include "IBufferedConnectionWriter.h"

ConnectionWriteStream::ConnectionWriteStream(const std::shared_ptr<IBufferedConnectionWriter>& writer, unsigned char virtid, bool blocking)
	: _writer(writer)
	, _virtid(virtid)
	, _blocking(blocking)
{
}

unsigned ConnectionWriteStream::maxPacketLength() const
{
	return _writer->capacity();
}

int ConnectionWriteStream::write(const char* buff, unsigned length)
{
	return _writer->write(_virtid, buff, length, _blocking);
}

bool ConnectionWriteStream::flush()
{
	return _writer->flush(_blocking);
}