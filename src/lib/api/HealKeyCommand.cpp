/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "HealKeyCommand.h"

#include "deskew/ICorrectSkew.h"

HealKeyCommand::HealKeyCommand(ICorrectSkew& corrector)
	: _corrector(corrector)
{
}

bool HealKeyCommand::run(const char*, unsigned)
{
	_corrector.healKey(*_peer, params, params.key);
	return true;
}

Turbopump::Request* HealKeyCommand::request()
{
	return &params;
}
