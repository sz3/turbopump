/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ReadCommand.h"

#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "socket/IByteStream.h"
#include <vector>

ReadCommand::ReadCommand(const IDataStore& dataStore, IByteStream& writer)
	: _dataStore(dataStore)
	, _writer(writer)
{
}

bool ReadCommand::run(const char*, unsigned)
{
	IDataStoreReader::ptr reader;
	if (params.version.empty())
	{
		std::vector<IDataStoreReader::ptr> reads = _dataStore.read(params.name);
		if (!reads.empty())
			reader = reads.front();
	}
	else
		reader = _dataStore.read(params.name, params.version);

	if (!reader)
		return false;
	while (reader->read(_writer) > 0);
	return true;
}

Turbopump::Request* ReadCommand::request()
{
	return &params;
}
