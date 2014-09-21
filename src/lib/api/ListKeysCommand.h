#pragma once

#include "ListKeys.h"

#include "Command.h"
#include <string>

class IByteStream;
class IDataStore;

class ListKeysCommand : public Command
{
public:
	ListKeysCommand(const IDataStore& dataStore, IByteStream& writer);

	bool run(const DataBuffer& data);
	Turbopump::Request* request();

protected:
	const IDataStore& _dataStore;
	IByteStream& _writer;

public:
	Turbopump::ListKeys params;
};
