/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "readstream.h"
#include "common/KeyMetadata.h"
#include <memory>
class IWriter;

class writestream 
{
public:
	writestream(IWriter* writer, const KeyMetadata& md);
	bool good() const;

	int write(const char* buffer, unsigned length);
	bool flush();
	bool close();

	readstream reader() const;

protected:
	std::shared_ptr<IWriter> _writer;
	KeyMetadata _md;
};
