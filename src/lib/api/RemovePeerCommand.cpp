/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "RemovePeerCommand.h"

#include "Api.h"
#include "Write.h"
#include "common/turbopump_defaults.h"
#include "common/VectorClock.h"

RemovePeerCommand::RemovePeerCommand(const Turbopump::Api& api)
	: _api(api)
{
}

bool RemovePeerCommand::run(const char*, unsigned)
{
	VectorClock version;
	version.increment(params.uid);
	version.markDeleted();

	Turbopump::Write req;
	req.name = MEMBERSHIP_FILE_PREFIX + params.uid;
	req.version = version.toString();
	req.copies = 0;

	std::unique_ptr<Turbopump::Command> cmd(_api.command(req));
	if( !cmd->run() )
		return setStatus(cmd->status());

	cmd->run(); // commit
	return setStatus(cmd->status());
}

Turbopump::Request* RemovePeerCommand::request()
{
	return &params;
}
