/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IWriter.h"
#include "readstream.h"

#include "common/KeyMetadata.h"
#include "serialize/StringUtil.h"
#include <functional>
#include <memory>

class writestream 
{
public:
	writestream();
	writestream(IWriter* writer, const KeyMetadata& md, std::function<bool(KeyMetadata&)> onClose = NULL);
	bool good() const;
	unsigned long long position() const;

	int write(const char* buffer, unsigned length);
	readstream commit(bool close=false);

	static unsigned long long digest(const std::string& version, unsigned long long size);

protected:
	std::shared_ptr<IWriter> _writer;
	KeyMetadata _md;
	std::function<bool(KeyMetadata&)> _onClose;
};

inline writestream::writestream()
	: _writer(NULL)
	, _onClose(NULL)
{
}

inline writestream::writestream(IWriter* writer, const KeyMetadata& md, std::function<bool(KeyMetadata&)> onClose)
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

// always flush. Sometimes close. If we're trying to close, the onClose callback is fired.
// _onClose *MAY* augment the _md.digest, if the close operation results in removing other versions of this key.
inline readstream writestream::commit(bool close)
{
	if ( !_writer->flush() )
		return readstream();
	if (close && !_writer->close() )
		return readstream();

	IReader* reader = _writer->reader();
	// TODO: _writer should compute size himself, rather than us going to reader->size()
	_md.digest = digest(_md.version.toString(), reader->size());
	if (close && _onClose)
		_onClose(_md);
	return readstream(reader, _md);
}

// used by the merkle/digest tree to determine if we have this file or not.
// this will hopefully be a cryptographic hash of the file contents.
// at the very least, it should include a crc.
// But for now... stir the version and size together into a 64 bit long.
inline unsigned long long writestream::digest(const std::string& version, unsigned long long size)
{
	// NOTE: this is meant to be xor'd with the digest of the thing we're replacing... which might be an older version of this version.
	//  see WriteCommand, Callbacks, and DigestTree.
	std::string dig = version + StringUtil::str(size);
	return Hash(dig).integer();
}
