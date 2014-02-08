/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "LocalWriteAction.h"

#include "consistent_hashing/ILocateKeys.h"
#include "data_store/IDataStore.h"

LocalWriteAction::LocalWriteAction(IDataStore& transientStore, IDataStore& dataStore, const ILocateKeys& locator, std::function<void(WriteParams, IDataStoreReader::ptr)> onCommit)
	: WriteAction(dataStore, onCommit)
	, _transientStore(transientStore)
	, _locator(locator)
{
}

IDataStoreWriter::ptr LocalWriteAction::open(const WriteParams& params)
{
	if (!_locator.keyIsMine(params.filename, params.totalCopies))
		return _transientStore.write(params.filename);
	return _dataStore.write(params.filename);
}
