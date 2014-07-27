/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "WriteAction.h"
class ILocateKeys;

class LocalWriteAction : public WriteAction
{
public:
	LocalWriteAction(IDataStore& transientStore, IDataStore& dataStore, const ILocateKeys& locator, std::function<void(WriteParams&, IDataStoreReader::ptr)> onCommit=NULL);

	IDataStoreWriter::ptr open(const WriteParams& params);

protected:
	IDataStore& _transientStore;
	const ILocateKeys& _locator;
};

