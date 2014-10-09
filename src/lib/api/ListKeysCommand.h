/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "ListKeys.h"

#include "StreamingCommand.h"
class IByteStream;
class IDataStore;

class ListKeysCommand : public StreamingCommand
{
public:
	ListKeysCommand(const IDataStore& dataStore);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	const IDataStore& _dataStore;

public:
	Turbopump::ListKeys params;
};
