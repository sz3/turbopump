/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ListKeys.h"

#include "Command.h"
#include <string>

class IByteStream;
class IDataStore;

class ListKeysCommand : public Turbopump::Command
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
