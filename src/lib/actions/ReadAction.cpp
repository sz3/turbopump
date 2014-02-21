/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ReadAction.h"

#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "socket/IByteStream.h"
#include <map>
#include <string>
#include <vector>
using std::map;
using std::string;
using std::vector;

ReadAction::ReadAction(IDataStore& dataStore, IByteStream& writer)
	: _dataStore(dataStore)
	, _writer(writer)
{
}

std::string ReadAction::name() const
{
	return "read";
}

bool ReadAction::run(const DataBuffer& data)
{
	IDataStoreReader::ptr reader;
	if (_version.empty())
	{
		vector<IDataStoreReader::ptr> reads = _dataStore.read(_filename);
		if (!reads.empty())
			reader = reads.front();
	}
	else
		reader = _dataStore.read(_filename, _version);

	if (!reader)
		return false;
	while (reader->read(_writer) > 0);
	return true;
}

void ReadAction::setParams(const map<string,string>& params)
{
	map<string,string>::const_iterator it = params.find("name");
	if (it != params.end())
		_filename = it->second;

	it = params.find("version");
	if (it != params.end())
		_version = it->second;
}

