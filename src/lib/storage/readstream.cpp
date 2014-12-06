/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "readstream.h"

#include "IReader.h"
#include "hashing/Hash.h"

#include "file/File.h"
#include "serialize/StringUtil.h"
#include "socket/IByteStream.h"

readstream::readstream()
{}

readstream::readstream(const std::shared_ptr<IReader>& reader, const KeyMetadata& md)
	: _reader(reader)
	, _md(md)
{
}

readstream::operator bool() const
{
	return good();
}

bool readstream::good() const
{
	return !!_reader && _reader->good();
}

int readstream::stream(IByteStream& sink)
{
	return _reader->stream(sink);
}

unsigned long long readstream::size() const
{
	return _reader->size();
}

// used by the merkle/digest tree to determine if we have this file or not.
// this will hopefully be a cryptographic hash of the file contents.
// at the very least, it should include a crc.
// But for now... stir the version and size together into a 64 bit long.
unsigned long long readstream::digest() const
{
	std::string dig = version() + StringUtil::str(size());
	return Hash(dig).integer();
}

unsigned short readstream::mirrors() const
{
	return _md.totalCopies;
}

std::string readstream::version() const
{
	return _md.version.toString();
}
