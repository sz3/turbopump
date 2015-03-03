/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "common/IStatusReporter.h"
#include "util/CallHistory.h"

class MockStatusReporter : public IStatusReporter
{
public:
	std::string status(const std::string& system) const
	{
		_history.call("status", system);
		return _status;
	}

public:
	std::string _status;
	mutable CallHistory _history;
};
