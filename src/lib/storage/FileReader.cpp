/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "FileReader.h"
#include "hashing/Hash.h"

#include "file/File.h"
#include "socket/IByteStream.h"
#include <iostream>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <attr/attributes.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace {
	unsigned long long file_size(int fd)
	{
		struct stat stat_buf;
		if (::fstat(fd, &stat_buf) != 0)
			return -1;
		return stat_buf.st_size;
	}
}

FileReader::FileReader(int fd)
	: _fd(fd)
{
}

FileReader::FileReader(const std::string& filename, unsigned long long offset)
	: _fd(-1)
{
	open(filename, offset);
}

FileReader::~FileReader()
{
	close();
}

void FileReader::close()
{
	if (good())
	{
		::close(_fd);
		_fd = -1;
	}
}

bool FileReader::open(const std::string& filename, unsigned long long offset)
{
	_fd = ::open(filename.c_str(), O_RDONLY | O_NOATIME);
	if ( !good() )
		std::cout << "couldn't read file: " << filename << ", " << ::strerror(errno) << std::endl;
	if ( offset != 0 && !setPosition(offset) )
	{
		std::cout << "couldn't seek for read of: " << filename << ", " << ::strerror(errno) << std::endl;
		close();
	}
	return good();
}

bool FileReader::good() const
{
	return _fd != -1;
}

unsigned long long FileReader::size() const
{
	return file_size(_fd);
}

std::string FileReader::attribute(const char* key) const
{
	int buflen = 200;
	std::string value;
	value.resize(buflen);
	if (::attr_getf(_fd, key, &value[0], &buflen, 0) != 0)
		return "";
	value.resize(buflen);
	return value;
}

bool FileReader::setPosition(unsigned long long pos)
{
	return ::lseek64(_fd, pos, SEEK_SET) != -1;
}

int FileReader::stream(IByteStream& sink)
{
	// somehow, optional unsigned long long limit passed in from the FileWriter
	char buff[sink.maxPacketLength()];
	unsigned bytesRead = ::read(_fd, buff, sink.maxPacketLength());
	if (bytesRead == 0)
		return 0;
	return sink.write(buff, bytesRead);
}
