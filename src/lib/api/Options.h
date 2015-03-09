/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "util/function_chain.h"

namespace Turbopump { class Drop; }
class WriteInstructions;
class readstream;

namespace Turbopump {
struct Options
{
	short internal_port = 9001;

	bool partition_keys = true;
	bool write_chaining = true;
	bool active_sync = true;
	bool udt = true;

	unsigned sync_interval_ms = 2000;
	unsigned offload_interval_ms = 5000;

	turbo::function_chain<WriteInstructions&, readstream&> when_local_write_finishes;
	turbo::function_chain<WriteInstructions&, readstream&> when_mirror_write_finishes;
	turbo::function_chain<const Turbopump::Drop&>          when_drop_finishes;
};
}//namespace

