/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SyncCommand.h"

#include "deskew/ISynchronize.h"
#include <string>
using std::string;

SyncCommand::SyncCommand(ISynchronize& sync)
	: _sync(sync)
{
}

bool SyncCommand::run(const char* buff, unsigned size)
{
	if (size == 0)
		return false;

	std::vector<string> points = StringUtil::split(string(buff,size), '|');
	for (unsigned i = 0; i < points.size(); ++i)
	{
		MerklePoint point;
		if (!MerklePointSerializer::fromString(point, points[i]))
			return false;

		_sync.compare(*_peer, params, point, i >= 2);
	}
	return true;
}

Turbopump::Request* SyncCommand::request()
{
	return &params;
}
