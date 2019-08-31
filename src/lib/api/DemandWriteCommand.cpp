/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DemandWriteCommand.h"

#include "deskew/ICorrectSkew.h"

#include "serialize/format.h"
#include <iostream>

DemandWriteCommand::DemandWriteCommand(ICorrectSkew& corrector)
	: _corrector(corrector)
{
}

bool DemandWriteCommand::run(const char*, unsigned)
{
	// how demanding. You want a key, but you won't tell us what its name is!
	if (params.name.empty())
		return false;

	std::cerr << fmt::format("logger: DemandWriteCommand {} : {} : {} : {}", params.name, params.version, params.offset, params.source) << std::endl;

	// TODO: sendKey should fail if offset >= length... or any other error, tbh
	_corrector.sendKey(*_peer, params.name, params.version, params.offset, params.source);
	return true;
}

Turbopump::Request* DemandWriteCommand::request()
{
	return &params;
}
