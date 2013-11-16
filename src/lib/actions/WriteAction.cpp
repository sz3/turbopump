#include "WriteAction.h"

#include "common/DataBuffer.h"
#include "data_store/IDataStore.h"
#include <map>
#include <string>
using std::map;
using std::string;

// because of UDP, I think this ought to be a push model. That is, the (stream) socket layer maintains offset state,
// and *pushes* data into the datastore. WriteActions would thus live across multiple buffers, updating the offset
// accordingly.
// and probably not taking the params on their run() function...
WriteAction::WriteAction(IDataStore& dataStore, std::function<void(std::string, IDataStoreReader::ptr)> onCommit)
	: _dataStore(dataStore)
	, _onCommit(std::move(onCommit))
	, _started(false)
	, _finished(false)
{
}

WriteAction::~WriteAction()
{
	if (!_finished)
		commit();
}

bool WriteAction::finished() const
{
	return _finished;
}

bool WriteAction::commit()
{
	_finished = true;
	IDataStoreReader::ptr reader = _writer->commit();
	if (!reader)
		return false;

	if (_onCommit)
		_onCommit(_filename, reader);
	return true;
}

std::string WriteAction::name() const
{
	return "write";
}

// now: [data]
// eventually: [8 byte offset][data]
bool WriteAction::run(const DataBuffer& data)
{
	if (_finished)
		return false;
	if (data.size() == 0)
	{
		// special case: don't commit() for a 0-byte first packet
		if (!_started)
			return _started = true;
		return commit();
	}

	_started |= true;
	_writer->write(data.buffer(), data.size());
	return true;
}

void WriteAction::setParams(const map<string,string>& params)
{
	map<string,string>::const_iterator it = params.find("name");
	if (it != params.end())
	{
		_filename = it->second;
		_writer = _dataStore.write(_filename);
	}
}

bool WriteAction::good() const
{
	return (bool)_writer;
}
