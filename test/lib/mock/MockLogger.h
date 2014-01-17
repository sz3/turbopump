/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "logging/ILog.h"
#include "util/CallHistory.h"

class MockLogger : public ILog
{
public:
	void logDebug(const std::string& msg);
	void logError(const std::string& msg);
	void logTrace(const std::string& msg);
	void logWarn(const std::string& msg);

public:
	CallHistory _history;
};
