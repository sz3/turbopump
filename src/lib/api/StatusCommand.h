/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "Status.h"

#include "Command.h"
class IByteStream;
class IStatusReporter;

class StatusCommand : public Turbopump::Command
{
public:
	StatusCommand(const IStatusReporter& reporter, IByteStream& writer, const std::string& view="");

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	const IStatusReporter& _reporter;
	IByteStream& _writer;

public:
	Turbopump::Status params;
};
