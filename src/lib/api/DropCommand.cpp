/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DropCommand.h"

#include "hashing/ILocateKeys.h"
#include "storage/IStore.h"
#include "storage/readstream.h"

DropCommand::DropCommand(IStore& store, const ILocateKeys& locator, std::function<void(const Turbopump::Drop&)> onDrop)
	: _store(store)
	, _locator(locator)
	, _onDrop(onDrop)
{
}

bool DropCommand::run(const char*, unsigned)
{
	{
		readstream reader = _store.read(params.name);
		if (!reader)
			return setStatus(404);

		params.copies = reader.mirrors();
		if (_locator.keyIsMine(params.name, params.copies))
			return setStatus(400);
	}

	if (!_store.remove(params.name))
		return setStatus(500);

	if (_onDrop)
		_onDrop(params);
	return setStatus(200);
}

Turbopump::Request* DropCommand::request()
{
	return &params;
}
