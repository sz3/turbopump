#include "MockStoreWriter.h"

#include "util/CallHistory.h"

namespace {
	CallHistory _history;
}

bool MockStoreWriter::good() const
{
	return true;
}

unsigned long long MockStoreWriter::position() const
{
	return _position;
}

int MockStoreWriter::write(const char* buffer, unsigned length)
{
	_history.call("write", std::string(buffer, length));
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
	return _reader;
}

std::string MockStoreWriter::calls()
{
	std::string cs = _history.calls();
	_history.clear();
	return cs;
}
