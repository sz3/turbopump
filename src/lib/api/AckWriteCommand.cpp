/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AckWriteCommand.h"

#include "deskew/ICorrectSkew.h"
#include "storage/IStore.h"
#include "storage/readstream.h"
#include "hashing/ILocateKeys.h"

AckWriteCommand::AckWriteCommand(ICorrectSkew& corrector, IStore& store, const ILocateKeys& locator)
	: _corrector(corrector)
	, _store(store)
	, _locator(locator)
{
}

bool AckWriteCommand::run(const char*, unsigned)
{
	readstream reader = _store.read(params.name, params.version);
	if (!reader)
		return false;

	// right now, we don't do anything unless we've been ack'd the whole file
	if (reader.size() != params.size)
		return false;

	unsigned short totalCopies = reader.mirrors();
	if (!_locator.keyIsMine(params.name, totalCopies))
		_corrector.dropKey(params.name);
	return true;
}

Turbopump::Request* AckWriteCommand::request()
{
	return &params;
}
