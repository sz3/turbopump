/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Command.h"
#include "util/CallHistory.h"

class MockCommand : public Turbopump::Command
{
public:
	bool run(const char* buff=NULL, unsigned size=0)
	{
		if (size == 0)
			_history.call("run");
		else
			_history.call("run", std::string(buff, size));
		return true;
	}

	Turbopump::Request* request()
	{
		return NULL;
	}

public:
	CallHistory _history;
};
