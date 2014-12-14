/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ReadCommand.h"

#include "storage/IStore.h"
#include "storage/readstream.h"

#include "socket/IByteStream.h"
#include <vector>

ReadCommand::ReadCommand(const IStore& store)
	: _store(store)
{
}

bool ReadCommand::run(const char*, unsigned)
{
	if (!_stream)
		return false;

	readstream reader = _store.read(params.name, params.version);
	if (!reader)
		return setStatus(404);
	while (reader.stream(*_stream) > 0);
	return setStatus(200);
}

Turbopump::Request* ReadCommand::request()
{
	return &params;
}
