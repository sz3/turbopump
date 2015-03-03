/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "StatusCommand.h"

#include "common/IStatusReporter.h"
#include "socket/IByteStream.h"

StatusCommand::StatusCommand(const IStatusReporter& reporter, const std::string& view)
	: _reporter(reporter)
{
	params.view = view;
}

bool StatusCommand::run(const char*, unsigned)
{
	if (!_stream)
		return false;

	std::string res = _reporter.status(params.view);
	_stream->write(res.data(), res.size());
	return true;
}

Turbopump::Request* StatusCommand::request()
{
	return &params;
}
