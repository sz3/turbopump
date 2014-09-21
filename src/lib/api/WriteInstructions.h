/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Write.h"
#include "common/turbopump_defaults.h"
#include <memory>
#include <string>
class ConnectionWriteStream;

struct WriteInstructions : public Turbopump::Write
{
	std::shared_ptr<ConnectionWriteStream> outstream;
	bool isComplete = 0;
};
