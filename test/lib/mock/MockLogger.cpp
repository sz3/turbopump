/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockLogger.h"

void MockLogger::logDebug(const std::string& msg)
{
	_history.call("logDebug", msg);
}

void MockLogger::logError(const std::string& msg)
{
	_history.call("logError", msg);
}

void MockLogger::logWarn(const std::string& msg)
{
	_history.call("logWarn", msg);
}
