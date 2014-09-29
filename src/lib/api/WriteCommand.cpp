/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WriteCommand.h"

#include "data_store/DataEntry.h"
#include "data_store/IDataStore.h"
#include <map>
#include <string>
using std::map;
using std::string;

// because of UDP, I think this ought to be a push model. That is, the (stream) socket layer maintains offset state,
// and *pushes* data into the datastore. WriteCommands would thus live across multiple buffers, updating the offset
// accordingly.
// and probably not taking the params on their run() function...
WriteCommand::WriteCommand(IDataStore& dataStore, std::function<void(WriteInstructions&, IDataStoreReader::ptr)> onCommit)
	: _dataStore(dataStore)
	, _onCommit(std::move(onCommit))
	, _started(false)
	, _finished(false)
	, _bytesSinceLastFlush(0)
{
}

WriteCommand::~WriteCommand()
{
	if (!_finished && _writer)
		commit();
}

Turbopump::Request* WriteCommand::request()
{
	return &_instructions;
}

bool WriteCommand::finished() const
{
	return _finished;
}

bool WriteCommand::flush()
{
	IDataStoreReader::ptr reader = _writer->commit();
	if (!reader)
		return false;

	if (_instructions.version.empty())
		_instructions.version = reader->metadata().version.toString();
	if (_onCommit)
		_onCommit(_instructions, reader);

	_instructions.offset = reader->size();
	_bytesSinceLastFlush = 0;
	return true;
}

bool WriteCommand::commit()
{
	_finished = true;
	_instructions.isComplete = true;
	if ( !flush() )
		return setStatus(500);

	_writer.reset();
	_instructions.outstream.reset();
	return setStatus(200);
}

// now: [data]
// eventually: [1 byte packet number][data]
// ... multiply w/ packet size to get the offset
// ... to go along with a size on write init to determine close
// ... may have to split this into local write and remote write parts. :(
bool WriteCommand::run(const char* buff, unsigned size)
{
	if (_finished)
		return false;

	if (!_started)
	{
		if (_instructions.name.empty())
		{
			_finished = true;
			return setStatus(400);
		}
		_writer = _dataStore.write(_instructions.name);
		_writer->setOffset(_instructions.offset);
		_writer->metadata().totalCopies = _instructions.copies;
		_writer->metadata().version.fromString(_instructions.version);
	}

	if (buff == NULL || size == 0)
	{
		// special case: don't commit() for a 0-byte first packet
		if (!_started)
			return _started = true;
		return commit();
	}

	_started |= true;

	// TODO: if we end up having to separate flush() and commit()/close() into separate calls in IDataStoreWriter anyway,
	//   then arguably we can also be more sloppy about what we write. (since the hashing algorithm could be choosy and still do his 64k blocks)

	// wan: write to mirror (nonblocking, could fail -> get pushed into PendingWrites), then write (on same thread) to disk
	// local: pass buffer for write to disk (on other thread), write to mirror (blocking), then wait for disk write to finish

	unsigned nextSize = _bytesSinceLastFlush + size;
	if (nextSize > 0x10000) // need to flush()
	{
		unsigned overfill = nextSize - 0x10000;
		unsigned topoff = size - overfill;
		if (topoff > 0)
			_writer->write(buff, topoff);
		if ( !flush() )
			return false;
		_writer->write(buff+topoff, overfill);
		_bytesSinceLastFlush += overfill;
	}
	else
	{
		_writer->write(buff, size);
		_bytesSinceLastFlush = nextSize;
	}

	return true;
}
