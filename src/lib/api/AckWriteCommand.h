/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "AckWrite.h"

#include "Command.h"
class ICorrectSkew;
class ILocateKeys;
class IStore;

namespace Turbopump { class Drop; }

class AckWriteCommand : public Turbopump::Command
{
public:
	AckWriteCommand(ICorrectSkew& corrector, IStore& store, const ILocateKeys& locator);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

	bool drop(const Turbopump::Drop& params);

protected:
	ICorrectSkew& _corrector;
	IStore& _store;
	const ILocateKeys& _locator;

public:
	Turbopump::AckWrite params;
};
