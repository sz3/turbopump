/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "AckWrite.h"

#include "Command.h"
#include <functional>
class IDataStore;
class ILocateKeys;

namespace Turbopump { class Drop; }

class AckWriteCommand : public Turbopump::Command
{
public:
	AckWriteCommand(IDataStore& store, const ILocateKeys& locator, std::function<void(const Turbopump::Drop&)> onDrop=NULL);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

	bool drop(const Turbopump::Drop& params);

protected:
	IDataStore& _store;
	const ILocateKeys& _locator;
	std::function<void(const Turbopump::Drop&)> _onDrop;

public:
	Turbopump::AckWrite params;
};
