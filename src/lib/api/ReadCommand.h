/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "Read.h"

#include "StreamingCommand.h"
class IByteStream;
class IStore;

class ReadCommand : public StreamingCommand
{
public:
	ReadCommand(const IStore& store);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	const IStore& _store;

public:
	Turbopump::Read params;
};
