/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WaitCommand.h"

#include "callbacks/IWatches.h"
#include "storage/IStore.h"
#include "storage/readstream.h"

#include "socket/IByteStream.h"
#include "util/random.h"

WaitCommand::WaitCommand(const IStore& store, IWatches& watches)
	: _store(store)
	, _watches(watches)
{
}

bool WaitCommand::run(const char*, unsigned)
{
	if (!_stream)
		return false;

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

bool WaitCommand::tryRead()
{
	readstream reader = _store.read(params.name, params.version);
	if (!reader)
		return setStatus(404);

	std::string res = params.name;
	_stream->write(res.data(), res.size());
	return setStatus(200);
}

void WaitCommand::cancel()
{
	_watches.unwatch(params.name, _watchId);
	_waiter.cancel();
}

Turbopump::Request* WaitCommand::request()
{
	return &params;
}
