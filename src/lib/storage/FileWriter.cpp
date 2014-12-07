/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "FileWriter.h"

#include "FileReader.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

FileWriter::FileWriter(const std::string& filename, std::function<bool()> onClose)
	: _fd(-1)
	, _onClose(onClose)
{
	open(filename);
}

FileWriter::~FileWriter()
{
	close_internal();
}

void FileWriter::close_internal()
{
	if ( good() )
	{
		::close(_fd);
		_fd = -1;
	}
}

bool FileWriter::open(const std::string& filename)
{
	_fd = ::open(filename.c_str(), O_RDWR | O_CREAT | O_NOATIME, S_IRWXU);
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

int FileWriter::write(const char* buffer, unsigned length)
{
	return ::write(_fd, buffer, length);
}

bool FileWriter::flush()
{
	return true;
}

// mark as completed. Means a rename, ultimately, but: call the callback!
bool FileWriter::close()
{
	close_internal();
	if (!_onClose)
		return false;
	return _onClose();
}

IReader* FileWriter::reader() const
{
	// rather than ::dup() each time, I'd like to do it once.
	return new FileReader(::dup(_fd));
}

