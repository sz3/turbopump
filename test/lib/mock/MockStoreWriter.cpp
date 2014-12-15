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
	return _position;
}

int MockStoreWriter::write(const char* buffer, unsigned length)
{
	_history.call("write", std::string(buffer, length));
	_position += length;
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
