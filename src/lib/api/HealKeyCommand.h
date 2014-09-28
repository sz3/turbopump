/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "HealKey.h"

#include "InternalCommand.h"
class ICorrectSkew;

class HealKeyCommand : public InternalCommand
{
public:
	HealKeyCommand(ICorrectSkew& corrector);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	ICorrectSkew& _corrector;

public:
	Turbopump::HealKey params;
};
