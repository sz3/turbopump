/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "DemandWrite.h"

#include "InternalCommand.h"
class ICorrectSkew;

class DemandWriteCommand : public InternalCommand
{
public:
	DemandWriteCommand(ICorrectSkew& corrector);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	ICorrectSkew& _corrector;

public:
	Turbopump::DemandWrite params;
};
