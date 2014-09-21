#pragma once

#include <string>
namespace Turbopump { class Request; }

class Op
{
public:
	virtual ~Op() {}

	virtual bool run() = 0;
	virtual Turbopump::Request* request() = 0;

	std::string status() const
	{
		return _status;
	}

protected:
	std::string _status;
};
