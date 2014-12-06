/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IWriter.h"
#include <functional>
#include <string>

class FileWriter : public IWriter
{
public:
	FileWriter(const std::string& filename, std::function<bool()> onClose = NULL);
	~FileWriter();

	bool good() const;
	int write(const char* buffer, unsigned length);
	bool flush();
	bool close();

	std::shared_ptr<IReader> reader() const;

protected:
	bool open(const std::string& filename); //TODO: offset
	void close_internal();

protected:
	FILE* _fd;
	std::function<bool()> _onClose;
};
