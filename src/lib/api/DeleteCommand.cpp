/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DeleteCommand.h"

#include "Api.h"
#include "Write.h"
#include "common/VectorClock.h"

DeleteCommand::DeleteCommand(const Turbopump::Api& api)
	: _api(api)
{
}

bool DeleteCommand::run(const char*, unsigned)
{
	VectorClock version;
	version.fromString(params.version);
	version.markDeleted();

	Turbopump::Write req;
	req.name = params.name;
	req.version = version.toString();
	std::unique_ptr<Turbopump::Command> cmd(_api.command(req));

	std::string timestamp("timestamp");
	return cmd->run(timestamp.data(), timestamp.size());
}

Turbopump::Request* DeleteCommand::request()
{
	return &params;
}
