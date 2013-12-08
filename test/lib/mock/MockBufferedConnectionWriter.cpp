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
