/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AddPeerCommand.h"

#include "Api.h"
#include "Write.h"
#include "common/turbopump_defaults.h"
#include "common/VectorClock.h"

AddPeerCommand::AddPeerCommand(const Turbopump::Api& api)
	: _api(api)
{
}

bool AddPeerCommand::run(const char*, unsigned)
{
	VectorClock version;
	version.increment(params.uid);

	Turbopump::Write req;
	req.name = MEMBERSHIP_FILE_PREFIX + params.uid;
	req.version = version.toString();
	req.copies = 0;

	std::unique_ptr<Turbopump::Command> cmd(_api.command(req));
	return cmd->run(params.ip.data(), params.ip.size());
}

Turbopump::Request* AddPeerCommand::request()
{
	return &params;
}
