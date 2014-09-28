/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "KeyRequest.h"

#include "InternalCommand.h"
class ICorrectSkew;

class KeyRequestCommand : public InternalCommand
{
public:
	KeyRequestCommand(ICorrectSkew& corrector);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	ICorrectSkew& _corrector;

public:
	Turbopump::KeyRequest params;
};
