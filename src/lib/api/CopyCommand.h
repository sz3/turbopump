/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Command.h"

#include "Copy.h"
#include <functional>
class IStore;
class readstream;
class WriteInstructions;

class CopyCommand : public Turbopump::Command
{
public:
	CopyCommand(IStore& store, std::function<void(WriteInstructions&, readstream&)> onCommit=NULL);
	~CopyCommand();

	Turbopump::Request* request();
	bool run(const char* buff=NULL, unsigned size=0);

protected:
	IStore& _store;
	std::function<void(WriteInstructions&, readstream&)> _onCommit;

	Turbopump::Copy params;
};
