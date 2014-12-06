/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "FileWriter.h"

#include "FileReader.h"
#include <cstdio>

FileWriter::FileWriter(const std::string& filename, std::function<bool()> onClose)
	: _fd(NULL)
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
		::fclose(_fd);
		_fd = NULL;
	}
}

bool FileWriter::open(const std::string& filename)
{
	_fd = ::fopen(filename.c_str(), "ab");
	return _fd != NULL;
}

bool FileWriter::good() const
{
	return _fd != NULL;
}

int FileWriter::write(const char* buffer, unsigned length)
{
	return ::fwrite(buffer, sizeof(char), length, _fd);
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

std::shared_ptr<IReader> FileWriter::reader() const
{
	// FileReader needs to know when to stop!
	// FileWriter needs to hold more information... maybe?
	// ... better yet, WriteStream holds a shared_ptr<IWriter>, and handles all the boring crap himself?
	return NULL;
}

