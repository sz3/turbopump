/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "StatusCommand.h"

#include "main/IStatusReporter.h"
#include "socket/IByteStream.h"

StatusCommand::StatusCommand(const IStatusReporter& reporter, IByteStream& writer, const std::string& view)
	: _reporter(reporter)
	, _writer(writer)
{
	params.view = view;
}

bool StatusCommand::run(const char*, unsigned)
{
	std::string res = _reporter.status(params.view);
	_writer.write(res.data(), res.size());
	return true;
}

Turbopump::Request* StatusCommand::request()
{
	return &params;
}
