/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "Status.h"

#include "StreamingCommand.h"
class IByteStream;
class IStatusReporter;

class StatusCommand : public StreamingCommand
{
public:
	StatusCommand(const IStatusReporter& reporter, const std::string& view="");

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	const IStatusReporter& _reporter;

public:
	Turbopump::Status params;
};
