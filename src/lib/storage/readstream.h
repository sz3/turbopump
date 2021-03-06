/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IReader.h"
#include "common/KeyMetadata.h"
#include "hashing/Hash.h"

#include "file/File.h"
#include "serialize/str.h"
#include "socket/IByteStream.h"
#include <memory>
#include <string>

class readstream
{
public:
	readstream();
	readstream(IReader* reader, const KeyMetadata& md);

	operator bool() const;
	bool good() const;

	bool setPosition(unsigned long long offset);
	int stream(IByteStream& sink);

	unsigned long long size() const;
	unsigned long long digest() const;
	unsigned short mirrors() const;
	std::string version() const;

protected:
	std::shared_ptr<IReader> _reader;
	KeyMetadata _md;
};

inline readstream::readstream()
{}

inline readstream::readstream(IReader* reader, const KeyMetadata& md)
	: _reader(reader)
	, _md(md)
{
}

inline readstream::operator bool() const
{
	return good();
}

inline bool readstream::good() const
{
	return !!_reader && _reader->good();
}

inline bool readstream::setPosition(unsigned long long offset)
{
	return _reader->setPosition(offset);
}

inline int readstream::stream(IByteStream& sink)
{
	return _reader->stream(sink);
}

inline unsigned long long readstream::size() const
{
	return _reader->size();
}

inline unsigned long long readstream::digest() const
{
	return _md.digest;
}

inline unsigned short readstream::mirrors() const
{
	return _md.totalCopies;
}

inline std::string readstream::version() const
{
	return _md.version.toString();
}
