/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "Read.h"

#include "StreamingCommand.h"
#include "concurrent/monitor.h"
class IStore;
class IWatches;

class ReadCommand : public StreamingCommand
{
public:
	ReadCommand(const IStore& store, IWatches& watches);

	bool run(const char* buff=NULL, unsigned size=0);
	void cancel();
	Turbopump::Request* request();

protected:
	bool tryRead();

protected:
	const IStore& _store;
	IWatches& _watches;
	std::string _watchId;
	turbo::monitor _waiter;

public:
	Turbopump::Read params;
};
