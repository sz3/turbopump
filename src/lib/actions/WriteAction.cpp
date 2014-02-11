/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WriteAction.h"

#include "common/DataBuffer.h"
#include "data_store/DataEntry.h"
#include "data_store/IDataStore.h"
#include <map>
#include <string>
using std::map;
using std::string;

// because of UDP, I think this ought to be a push model. That is, the (stream) socket layer maintains offset state,
// and *pushes* data into the datastore. WriteActions would thus live across multiple buffers, updating the offset
// accordingly.
// and probably not taking the params on their run() function...
WriteAction::WriteAction(IDataStore& dataStore, std::function<void(WriteParams, IDataStoreReader::ptr)> onCommit)
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
		_onCommit(_params, reader);
	_writer.reset();
	return true;
}

std::string WriteAction::name() const
{
	return "write";
}

// now: [data]
// eventually: [1 byte packet number][data]
// ... multiply w/ packet size to get the offset
// ... to go along with a size on write init to determine close
// ... may have to split this into local write and remote write parts. :(
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
	_params.mirror = 0;

	map<string,string>::const_iterator it = params.find("i");
	if (it != params.end())
		_params.mirror = std::stoi(it->second);

	it = params.find("n");
	if (it != params.end())
		_params.totalCopies = std::stoi(it->second);

	it = params.find("source");
	if (it != params.end())
		_params.source = it->second;

	it = params.find("name");
	if (it != params.end())
	{
		_params.filename = it->second;
		_writer = open(_params);
		_writer->metadata().totalCopies = _params.totalCopies;
	}
}

// virtual method. See LocalWriteAction::open() for an alternate take
IDataStoreWriter::ptr WriteAction::open(const WriteParams& params)
{
	// if !params.version.empty()
	// return _dataStore.write(params.filename, params.version);
	return _dataStore.write(params.filename);
}

bool WriteAction::multiPacket() const
{
	return true;
}

bool WriteAction::good() const
{
	return (bool)_writer;
}
