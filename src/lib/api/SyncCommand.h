/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "Sync.h"

#include "InternalCommand.h"
class ISynchronize;

class SyncCommand : public InternalCommand
{
public:
	SyncCommand(ISynchronize& sync);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	ISynchronize& _sync;

public:
	Turbopump::Sync params;
};
