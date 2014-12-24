/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WriteCommand.h"

#include "storage/IStore.h"
#include <map>
#include <string>
using std::map;
using std::string;

// because of UDP, I think this ought to be a push model. That is, the (stream) socket layer maintains offset state,
// and *pushes* data into the datastore. WriteCommands would thus live across multiple buffers, updating the offset
// accordingly.
// and probably not taking the params on their run() function...
WriteCommand::WriteCommand(IStore& store, std::function<void(WriteInstructions&, readstream&)> onCommit)
	: _store(store)
	, _onCommit(std::move(onCommit))
	, _started(false)
	, _finished(false)
	, _bytesSinceLastFlush(0)
{
}

WriteCommand::~WriteCommand()
{
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
	readstream reader = _writer.commit(_instructions.isComplete);
	if (!reader)
		return false;

	if (_instructions.version.empty())
		_instructions.version = reader.version();
	if (_onCommit)
		_onCommit(_instructions, reader);

	// TODO: vvvvvvvvvvvvv
	_instructions.digest = reader.digest();
	_instructions.offset = _writer.position();
	_bytesSinceLastFlush = 0;
	return true;
}

bool WriteCommand::commit()
{
	_finished = true;
	_instructions.isComplete = true;
	if ( !flush() )
		return setStatus(500);

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
		_writer = _store.write(_instructions.name, _instructions.version, _instructions.copies, _instructions.offset);
	}

	if (buff == NULL || size == 0)
	{
		// special case: don't commit() for a 0-byte first packet
		if (!_started)
			return _started = true;
		return commit();
	}

	_started |= true;

	// wan: write to mirror (nonblocking, could fail -> get pushed into PendingWrites), then write (on same thread) to disk
	// local: pass buffer for write to disk (on other thread), write to mirror (blocking), then wait for disk write to finish

	unsigned nextSize = _bytesSinceLastFlush + size;
	if (nextSize > 0x10000) // need to flush()
	{
		unsigned overfill = nextSize - 0x10000;
		unsigned topoff = size - overfill;
		if (topoff > 0)
			_writer.write(buff, topoff);
		if ( !flush() )
			return false;
		_writer.write(buff+topoff, overfill);
		_bytesSinceLastFlush += overfill;
	}
	else
	{
		_writer.write(buff, size);
		_bytesSinceLastFlush = nextSize;
	}

	return true;
}
