#include "ListKeysOp.h"

#include "common/turbopump_defaults.h"
#include "data_store/IDataStore.h"
using std::string;

ListKeysOp::ListKeysOp(const IDataStore& dataStore, IByteStream& writer)
	: _dataStore(dataStore)
	, _writer(writer)
{
}

bool ListKeysOp::run()
{
	string excludes = params.all? "" : MEMBERSHIP_FILE_PREFIX;
	_dataStore.report(_writer, params.deleted, excludes);
	return true;
}

Turbopump::Request* ListKeysOp::request()
{
	return &params;
}
