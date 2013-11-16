#include "ReadAction.h"

#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "socket/IByteStream.h"
#include <map>
#include <string>
using std::map;
using std::string;

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
	IDataStoreReader::ptr reader = _dataStore.read(_filename);
	if (!reader)
		return false;
	// while. Or something.
	// flow control???
	reader->read(_writer);
	return true;
}

void ReadAction::setParams(const map<string,string>& params)
{
	map<string,string>::const_iterator it = params.find("name");
	if (it != params.end())
		_filename = it->second;
}

