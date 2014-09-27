/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DemandWriteCommand.h"

#include "deskew/ICorrectSkew.h"

DemandWriteCommand::DemandWriteCommand(ICorrectSkew& corrector)
	: _corrector(corrector)
{
}

bool DemandWriteCommand::run(const char*, unsigned)
{
	// how demanding. You want a key, but you won't tell us what its name is!
	if (params.name.empty())
		return false;

	_corrector.sendKey(*_peer, params.name, params.version, params.source);
	return true;
}

Turbopump::Request* DemandWriteCommand::request()
{
	return &params;
}
