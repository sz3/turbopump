/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "Read.h"

#include "Command.h"
class IByteStream;
class IDataStore;

class ReadCommand : public Turbopump::Command
{
public:
	ReadCommand(const IDataStore& dataStore, IByteStream& writer);

	bool run(const DataBuffer& data);
	Turbopump::Request* request();

protected:
	const IDataStore& _dataStore;
	IByteStream& _writer;

public:
	Turbopump::Read params;
};
