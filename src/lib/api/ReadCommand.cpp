/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ReadCommand.h"

#include "callbacks/Watches.h"
#include "storage/IStore.h"
#include "storage/readstream.h"

#include "concurrent/monitor.h"
#include "socket/IByteStream.h"
#include "util/random.h"

ReadCommand::ReadCommand(const IStore& store, IWatches& watches)
	: _store(store)
	, _watches(watches)
{
}

bool ReadCommand::run(const char*, unsigned)
{
	if (!_stream)
		return false;

	if (params.wait)
	{
		_watchId = turbo::random::bytes(16);
		auto waitFun = [this] ()
		{
			this->_waiter.signal_all();
		};
		_watches.watch(params.name, _watchId, waitFun);

		bool res = tryRead();
		if (!res)
		{
			setStatus(0);
			if (_waiter.wait())
				res = tryRead();
		}

		_watches.unwatch(params.name, _watchId);
		return res;
	}
	else
		return tryRead();
}

bool ReadCommand::tryRead()
{
	readstream reader = _store.read(params.name, params.version);
	if (!reader)
		return setStatus(404);
	while (reader.stream(*_stream) > 0);
	return setStatus(200);
}

void ReadCommand::cancel()
{
	_watches.unwatch(params.name, _watchId);
	_waiter.cancel();
}

Turbopump::Request* ReadCommand::request()
{
	return &params;
}
