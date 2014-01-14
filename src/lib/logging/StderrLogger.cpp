/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "StderrLogger.h"

#include <ctime>
#include <iomanip>
#include <iostream>

StderrLogger::StderrLogger(const std::string& myid)
	: _myid(myid)
{
}

void StderrLogger::logDebug(const std::string& msg)
{
	logMessage("DEBUG", msg);
}

void StderrLogger::logError(const std::string& msg)
{
	logMessage("ERROR", msg);
}

void StderrLogger::logWarn(const std::string& msg)
{
	logMessage("WARN", msg);
}

void StderrLogger::logMessage(const std::string& type, const std::string& msg)
{
	std::time_t t;
	std::time(&t);
	std::cerr << _myid << "  " << std::ctime(&t) << "  " << msg << std::endl;
}