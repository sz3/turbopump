/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <functional>

namespace Turbopump { class Drop; }
class WriteInstructions;
class readstream;

namespace Turbopump {
struct Options
{
	bool partition_keys = true;
	bool write_chaining = true;
	bool active_sync = true;
	bool udt = true;

	std::function<void(WriteInstructions&, readstream& contents)> when_local_write_finishes;
	std::function<void(WriteInstructions&, readstream& contents)> when_mirror_write_finishes;
	std::function<void(const Turbopump::Drop&)>                   when_drop_finishes;
};
}//namespace

