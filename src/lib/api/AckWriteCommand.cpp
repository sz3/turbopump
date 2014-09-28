/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AckWriteCommand.h"

#include "Drop.h"
#include "common/KeyMetadata.h"
#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "hashing/ILocateKeys.h"

AckWriteCommand::AckWriteCommand(IDataStore& store, const ILocateKeys& locator, std::function<void(const Turbopump::Drop&)> onDrop)
	: _store(store)
	, _locator(locator)
	, _onDrop(onDrop)
{
}

bool AckWriteCommand::run(const char*, unsigned)
{
	IDataStoreReader::ptr reader = _store.read(params.name, params.version);
	if (!reader)
		return false;

	// right now, we don't do anything unless we've been ack'd the whole file
	if (reader->size() != params.size)
		return false;

	unsigned short totalCopies = reader->metadata().totalCopies;
	if (!_locator.keyIsMine(params.name, totalCopies))
	{
		Turbopump::Drop req;
		req.name = params.name;
		req.copies = totalCopies;
		drop(req);
	}
	return true;
}

bool AckWriteCommand::drop(const Turbopump::Drop& req)
{
	if (!_store.drop(req.name))
		return false;
	if (_onDrop)
		_onDrop(req);
	return true;
}

Turbopump::Request* AckWriteCommand::request()
{
	return &params;
}
