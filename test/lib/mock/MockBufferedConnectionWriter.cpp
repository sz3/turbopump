/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockBufferedConnectionWriter.h"

namespace {
	std::string boolStr(bool val)
	{
		if (val)
			return "true";
		else
			return "false";
	}
}

unsigned MockBufferedConnectionWriter::capacity() const
{
	return _capacity;
}

int MockBufferedConnectionWriter::write(unsigned char virtid, const char* buffer, unsigned length, bool blocking)
{
	_history.call("write", (unsigned)virtid, std::string(buffer, length), boolStr(blocking));
	return length;
}

bool MockBufferedConnectionWriter::flush(bool blocking)
{
	_history.call("flush", boolStr(blocking));
	return true;
}
