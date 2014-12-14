/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "ListKeys.h"

#include "StreamingCommand.h"
class IByteStream;
class IStore;

class ListKeysCommand : public StreamingCommand
{
public:
	ListKeysCommand(const IStore& store);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

	bool print_key(const std::string& report) const;

protected:
	const IStore& _store;

public:
	Turbopump::ListKeys params;
};
