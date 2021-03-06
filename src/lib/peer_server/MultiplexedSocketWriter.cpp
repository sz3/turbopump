/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MultiplexedSocketWriter.h"

#include "IBufferedConnectionWriter.h"
#include "socket/socket_address.h"

MultiplexedSocketWriter::MultiplexedSocketWriter(const std::shared_ptr<IBufferedConnectionWriter>& writer)
	: _muxid(0)
	, _writer(writer)
{
}

int MultiplexedSocketWriter::try_send(const char* buffer, unsigned size)
{
	return _writer->write(_muxid, buffer, size, false);
}

int MultiplexedSocketWriter::send(const char* buffer, unsigned size)
{
	return _writer->write(_muxid, buffer, size, true);
}

unsigned MultiplexedSocketWriter::capacity() const
{
	return _writer->capacity();
}

bool MultiplexedSocketWriter::flush(bool wait)
{
	return _writer->flush(wait);
}

// should also go away!
bool MultiplexedSocketWriter::close()
{
	return true;
}

// dummy, should go away
socket_address MultiplexedSocketWriter::endpoint() const
{
	return socket_address(target(), 0);
}

std::string MultiplexedSocketWriter::target() const
{
	return _writer->target();
}

int MultiplexedSocketWriter::handle() const
{
	return _writer->handle();
}

void MultiplexedSocketWriter::set_muxid(unsigned char id)
{
	_muxid = id;
}
