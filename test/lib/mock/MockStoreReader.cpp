/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockStoreReader.h"

#include "socket/IByteStream.h"
#include "util/CallHistory.h"

namespace {
	CallHistory _history;
}

bool MockStoreReader::good() const
{
	return true;
}

unsigned long long MockStoreReader::size() const
{
	return _size;
}

bool MockStoreReader::setPosition(unsigned long long offset)
{
	_history.call("setPosition", offset);
	return true;
}

int MockStoreReader::stream(IByteStream& sink)
{
	_history.call("stream");
	return sink.write("0123456789", 10);
}

std::string MockStoreReader::calls()
{
	std::string cs = _history.calls();
	_history.clear();
	return cs;
}
