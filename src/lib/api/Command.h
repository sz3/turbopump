#pragma once

#include "http/StatusCode.h"
#include <string>
class DataBuffer;
namespace Turbopump { class Request; }

class Command
{
public:
	virtual ~Command() {}

	virtual Turbopump::Request* request() = 0;
	virtual bool run(const DataBuffer& data) = 0;

	// should I hold onto this command, or can I throw it away?
	virtual bool finished() const
	{
		return true;
	}

	StatusCode status() const
	{
		return _status;
	}

protected:
	StatusCode _status;
};
