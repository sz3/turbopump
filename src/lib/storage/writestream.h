/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IWriter.h"
#include "readstream.h"

#include "common/KeyMetadata.h"
#include <functional>
#include <memory>

class writestream 
{
public:
	writestream();
	writestream(IWriter* writer, const KeyMetadata& md, std::function<bool(const KeyMetadata&)> onClose = NULL);
	bool good() const;
	unsigned long long position() const;

	int write(const char* buffer, unsigned length);
	bool flush();
	bool close();

	readstream reader() const;

protected:
	std::shared_ptr<IWriter> _writer;
	KeyMetadata _md;
	std::function<bool(const KeyMetadata&)> _onClose;
};

inline writestream::writestream()
	: _writer(NULL)
	, _onClose(NULL)
{
}

inline writestream::writestream(IWriter* writer, const KeyMetadata& md, std::function<bool(const KeyMetadata&)> onClose)
	: _writer(writer)
	, _md(md)
	, _onClose(onClose)
{
}

inline bool writestream::good() const
{
	return !!_writer && _writer->good();
}

inline unsigned long long writestream::position() const
{
	return _writer->position();
}

inline int writestream::write(const char* buffer, unsigned length)
{
	return _writer->write(buffer, length);
}

inline bool writestream::flush()
{
	return _writer->flush();
}

// mark as completed. Call the callback! For FileStore, this means a rename...
inline bool writestream::close()
{
	if ( !_writer->close() )
		return false;
	if (_onClose)
		return _onClose(_md);
	return true;
}

inline readstream writestream::reader() const
{
	return readstream(_writer->reader(), _md);
}
