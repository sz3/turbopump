/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "Drop.h"

#include "Command.h"
class ICorrectSkew;
class ILocateKeys;
class IStore;

class DropCommand : public Turbopump::Command
{
public:
	DropCommand(ICorrectSkew& corrector, IStore& store, const ILocateKeys& locator);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	ICorrectSkew& _corrector;
	IStore& _store;
	const ILocateKeys& _locator;

public:
	Turbopump::Drop params;
};
