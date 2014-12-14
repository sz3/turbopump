/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Write.h"
#include <memory>
class ConnectionWriteStream;

struct WriteInstructions : public Turbopump::Write
{
	WriteInstructions()
		: Turbopump::Write()
	{}

	WriteInstructions(const std::string& name, const std::string& version, short mirror, short copies)
		: Turbopump::Write()
	{
		this->name = name;
		this->version = version;
		this->mirror = mirror;
		this->copies = copies;
	}

	std::shared_ptr<ConnectionWriteStream> outstream;
	bool isComplete = false;
	unsigned long long digest = 0;
};
