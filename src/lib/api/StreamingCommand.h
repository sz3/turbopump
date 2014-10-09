/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Command.h"

class StreamingCommand : public Turbopump::Command
{
public:
	void setWriter(IByteStream* stream)
	{
		_stream = stream;
	}

protected:
	IByteStream* _stream = 0;
};
