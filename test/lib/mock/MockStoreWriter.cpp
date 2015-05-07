/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockStoreWriter.h"

#include "storage/StringReader.h"
#include "util/CallHistory.h"

namespace {
	CallHistory _history;
}

MockStoreWriter::MockStoreWriter()
{
	_history.clear();
}

bool MockStoreWriter::good() const
{
	return true;
}

unsigned long long MockStoreWriter::position() const
{
	return _reader.size();
}

int MockStoreWriter::write(const char* buffer, unsigned length)
{
	std::string data(buffer, length);
	_history.call("write", data);
	_reader += data;
	return length;
}

bool MockStoreWriter::flush()
{
	_history.call("flush");
	return true;
}

bool MockStoreWriter::close()
{
	_history.call("close");
	return true;
}

IReader* MockStoreWriter::reader() const
{
	_history.call("reader");
	return new StringReader(_reader);
}

std::string MockStoreWriter::calls()
{
	std::string cs = _history.calls();
	_history.clear();
	return cs;
}
