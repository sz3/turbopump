/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "FileWriter.h"

#include "FileReader.h"
#include <cstdio>

#include <fcntl.h>
#include <unistd.h>
#include <attr/attributes.h>
#include <sys/stat.h>
#include <sys/types.h>

FileWriter::FileWriter(const std::string& filename, bool append)
	: _fd(-1)
	, _filename(filename)
{
	open(append);
}

FileWriter::~FileWriter()
{
	close();
}

bool FileWriter::open(bool append)
{
	int flags = O_WRONLY | O_CREAT | O_NOATIME;
	if (append)
		flags |= O_APPEND;

	_fd = ::open(_filename.c_str(), flags, S_IRWXU);
	return good();
}

bool FileWriter::good() const
{
	return _fd != -1;
}

unsigned long long FileWriter::position() const
{
	return ::lseek64(_fd, 0, SEEK_CUR);
}

bool FileWriter::setAttribute(const char* key, const std::string& value)
{
	return ::attr_setf(_fd, key, value.data(), value.size(), 0) == 0;
}

bool FileWriter::link(const std::string& source)
{
	// obviously, it's optimal to call this without opening the file...
	if (good())
	{
		close();
		remove(_filename.c_str());
	}

	int res = ::link(source.c_str(), _filename.c_str());
	if (res != 0)
	{
		open();
		return false;
	}

	return true;
}

int FileWriter::write(const char* buffer, unsigned length)
{
	return ::write(_fd, buffer, length);
}

bool FileWriter::flush()
{
	// could do some fancy stuff with position() and reader's setPosition() here...
	return true;
}

bool FileWriter::close()
{
	if ( good() )
	{
		::close(_fd);
		_fd = -1;
	}
	return true;
}

IReader* FileWriter::reader() const
{
	// TODO: what are the tradeoffs of using _fd?
	return new FileReader(_filename);
}

