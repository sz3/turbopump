/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "Drop.h"

#include "Command.h"
#include <functional>
class ILocateKeys;
class IStore;

class DropCommand : public Turbopump::Command
{
public:
	DropCommand(IStore& store, const ILocateKeys& locator, std::function<void(const Turbopump::Drop&)> onDrop);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	IStore& _store;
	const ILocateKeys& _locator;
	std::function<void(const Turbopump::Drop&)> _onDrop;

public:
	Turbopump::Drop params;
};
