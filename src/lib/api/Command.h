/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "http/StatusCode.h"
#include <string>

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

	StatusCode status() const
	{
		return _status;
	}

	bool setError(int code)
	{
		_status = code;
		return false;
	}

protected:
	StatusCode _status;
};
}//namespace
