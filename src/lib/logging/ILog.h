/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

class ILog
{
public:
	virtual ~ILog() {}

	virtual void logDebug(const std::string& msg) = 0;
	virtual void logError(const std::string& msg) = 0;
	virtual void logWarn(const std::string& msg) = 0;
};
