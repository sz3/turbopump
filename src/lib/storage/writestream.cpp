/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "writestream.h"

#include "IWriter.h"

writestream::writestream(IWriter* writer, const KeyMetadata& md)
	: _writer(writer)
	, _md(md)
{
}

bool writestream::good() const
{
	return !!_writer && _writer->good();
}

int writestream::write(const char* buffer, unsigned length)
{
	return _writer->write(buffer, length);
}

bool writestream::flush()
{
	return _writer->flush();
}

// mark as completed. Means a rename, ultimately, but: call the callback!
bool writestream::close()
{
	return _writer->close();
}

readstream writestream::reader() const
{
	return readstream(_writer->reader(), _md);
}

