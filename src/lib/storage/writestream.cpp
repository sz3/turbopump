/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "writestream.h"

#include "IWriter.h"

writestream::writestream(IWriter* writer, const KeyMetadata& md, std::function<bool(const KeyMetadata&)> onClose)
	: _writer(writer)
	, _md(md)
	, _onClose(onClose)
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

// mark as completed. Call the callback! For FileStore, this means a rename...
bool writestream::close()
{
	if ( !_writer->close() )
		return false;
	if (_onClose)
		return _onClose(_md);
	return true;
}

readstream writestream::reader() const
{
	return readstream(_writer->reader(), _md);
}

