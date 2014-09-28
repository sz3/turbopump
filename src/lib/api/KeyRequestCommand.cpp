/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "KeyRequestCommand.h"

#include "deskew/ICorrectSkew.h"

KeyRequestCommand::KeyRequestCommand(ICorrectSkew& corrector)
	: _corrector(corrector)
{
}

bool KeyRequestCommand::run(const char*, unsigned)
{
	_corrector.pushKeyRange(*_peer, params, params.first, params.last);
	return true;
}

Turbopump::Request* KeyRequestCommand::request()
{
	return &params;
}
