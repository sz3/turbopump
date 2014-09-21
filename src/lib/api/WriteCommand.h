/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Command.h"

#include "WriteInstructions.h"
#include "data_store/IDataStoreReader.h"
#include "data_store/IDataStoreWriter.h"
#include <functional>

class IDataStore;

class WriteCommand : public Command
{
public:
	// instead of passing in a function, dedicate an interface to predefined functions, and have the params select from it?
	WriteCommand(IDataStore& dataStore, std::function<void(WriteInstructions&, IDataStoreReader::ptr)> onCommit=NULL);
	~WriteCommand();

	Turbopump::Request* request();
	bool run(const DataBuffer& data);
	bool finished() const;

protected:
	bool flush();
	bool commit();

protected:
	IDataStore& _dataStore;
	std::function<void(WriteInstructions&, IDataStoreReader::ptr)> _onCommit;
	bool _started;
	bool _finished;
	unsigned _bytesSinceLastFlush;

	WriteInstructions _instructions;
	IDataStoreWriter::ptr _writer;
};
