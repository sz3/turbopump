/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockBufferedConnectionWriter.h"

unsigned MockBufferedConnectionWriter::capacity() const
{
	return _capacity;
}

int MockBufferedConnectionWriter::write(unsigned char virtid, const char* buffer, unsigned length)
{
	_history.call("write", (unsigned)virtid, std::string(buffer, length));
	return length;
}

int MockBufferedConnectionWriter::flush()
{
	_history.call("flush");
	return 0;
}
