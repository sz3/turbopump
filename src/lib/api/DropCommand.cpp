/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DropCommand.h"

#include "data_store/DataEntry.h"
#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "hashing/ILocateKeys.h"

DropCommand::DropCommand(IDataStore& dataStore, const ILocateKeys& locator, std::function<void(const Turbopump::Drop&)> onDrop)
	: _dataStore(dataStore)
	, _locator(locator)
	, _onDrop(onDrop)
{
}

bool DropCommand::run(const char*, unsigned)
{
	{
		std::vector<IDataStoreReader::ptr> reads = _dataStore.read(params.name);
		if (reads.empty())
			return setStatus(400);

		params.copies = reads.front()->metadata().totalCopies;
		if (_locator.keyIsMine(params.name, params.copies))
			return setStatus(400);
	}

	if (!_dataStore.drop(params.name))
		return setStatus(500);

	if (_onDrop)
		_onDrop(params);
	return setStatus(200);
}

Turbopump::Request* DropCommand::request()
{
	return &params;
}
