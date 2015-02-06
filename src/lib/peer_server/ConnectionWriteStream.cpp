/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ConnectionWriteStream.h"

#include "socket/ISocketWriter.h"

ConnectionWriteStream::ConnectionWriteStream(const std::shared_ptr<ISocketWriter>& writer, bool blocking)
	: _writer(writer)
	, _blocking(blocking)
	, _full(false)
	, _background(false)
{
}

unsigned ConnectionWriteStream::maxPacketLength() const
{
	return _writer->capacity();
}

int ConnectionWriteStream::write(const char* buff, unsigned length)
{
	if (_blocking)
		return _writer->send(buff, length);

	int sent = _writer->try_send(buff, length);
	_full = (sent != length);
	return sent;
}

bool ConnectionWriteStream::flush()
{
	return _writer->flush(_blocking);
}

bool ConnectionWriteStream::full() const
{
	return _full;
}

void ConnectionWriteStream::setBackground(bool background)
{
	_background = background;
}

bool ConnectionWriteStream::background() const
{
	return _background;
}

ISocketWriter* ConnectionWriteStream::writer() const
{
	return _writer.get();
}
