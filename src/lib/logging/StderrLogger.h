/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ILog.h"

class StderrLogger : public ILog
{
public:
	StderrLogger(const std::string& myid);

	void logDebug(const std::string& msg);
	void logError(const std::string& msg);
	void logTrace(const std::string& msg);
	void logWarn(const std::string& msg);

protected:
	void logMessage(const std::string& type, const std::string& msg);

protected:
	std::string _myid;
};
