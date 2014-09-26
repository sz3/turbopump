/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"
#include <functional>

class WriteInstructions;

namespace Turbopump {
struct Options
{
	bool partition_keys = true;
	bool write_chaining = true;
	bool active_sync = true;
	bool udt = true;

	std::function<void(WriteInstructions&, IDataStoreReader::ptr contents)> when_local_write_finishes;
	std::function<void(WriteInstructions&, IDataStoreReader::ptr contents)> when_mirror_write_finishes;
};
}//namespace

