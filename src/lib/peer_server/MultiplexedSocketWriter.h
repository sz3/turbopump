/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "socket/ISocketWriter.h"
#include <memory>
class IBufferedConnectionWriter;

class MultiplexedSocketWriter : public ISocketWriter
{
public:
	MultiplexedSocketWriter(unsigned char muxid, const std::shared_ptr<IBufferedConnectionWriter>& writer);

	int try_send(const char* buffer, unsigned size);
	int send(const char* buffer, unsigned size);
	bool close();

	socket_address endpoint() const;
	std::string target() const;

protected:
	unsigned char _muxid;
	std::shared_ptr<IBufferedConnectionWriter> _writer;
};
