/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DropCommand.h"

#include "deskew/ICorrectSkew.h"
#include "hashing/ILocateKeys.h"
#include "storage/IStore.h"
#include "storage/readstream.h"

DropCommand::DropCommand(ICorrectSkew& corrector, IStore& store, const ILocateKeys& locator)
	: _corrector(corrector)
	, _store(store)
	, _locator(locator)
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

	if (!_corrector.dropKey(params.name))
		return setStatus(500);

	return setStatus(200);
}

Turbopump::Request* DropCommand::request()
{
	return &params;
}
