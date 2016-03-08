/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "http/StatusCode.h"
#include <memory>
class IByteStream;
class Peer;

namespace Turbopump {
class Request;

class Command
{
public:
	virtual ~Command() {}

	virtual Request* request() = 0;
	virtual bool run(const char* buff=NULL, unsigned size=0) = 0;

	// should I hold onto this command, or can I throw it away?
	virtual bool finished() const
	{
		return true;
	}

	// for commands with pending operations
	virtual void cancel() {}

	// TODO: should setPeer be a simple string?
	// please?
	virtual void setPeer(const std::shared_ptr<Peer>&) {}
	virtual void setWriter(IByteStream*) {}

	StatusCode status() const
	{
		return _status;
	}

	bool setStatus(int code)
	{
		_status = code;
		return _status < 400;
	}

protected:
	StatusCode _status;
};
}//namespace
