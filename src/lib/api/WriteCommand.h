/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Command.h"

#include "WriteInstructions.h"
#include "storage/writestream.h"

#include <functional>
class IStore;
class readstream;

class WriteCommand : public Turbopump::Command
{
public:
	// instead of passing in a function, dedicate an interface to predefined functions, and have the params select from it?
	WriteCommand(IStore& store, std::function<void(WriteInstructions&, readstream&)> onCommit=NULL);
	~WriteCommand();

	Turbopump::Request* request();
	bool run(const char* buff=NULL, unsigned size=0);
	bool finished() const;

protected:
	bool flush();
	bool commit();

protected:
	IStore& _store;
	std::function<void(WriteInstructions&, readstream&)> _onCommit;
	bool _started;
	bool _finished;
	unsigned _bytesSinceLastFlush;

	WriteInstructions _instructions;
	writestream _writer;
};
