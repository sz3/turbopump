/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ListKeysCommand.h"

#include "common/turbopump_defaults.h"
#include "data_store/IDataStore.h"
using std::string;

ListKeysCommand::ListKeysCommand(const IDataStore& dataStore, IByteStream& writer)
	: _dataStore(dataStore)
	, _writer(writer)
{
}

bool ListKeysCommand::run(const DataBuffer& data)
{
	string excludes = params.all? "" : MEMBERSHIP_FILE_PREFIX;
	_dataStore.report(_writer, params.deleted, excludes);
	return true;
}

Turbopump::Request* ListKeysCommand::request()
{
	return &params;
}
