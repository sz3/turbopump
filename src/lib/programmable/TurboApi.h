/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"
class DropParams;
class WriteParams;

struct TurboApi
{
	std::function<void(WriteParams&, IDataStoreReader::ptr contents)> when_local_write_finishes;
	std::function<void(WriteParams&, IDataStoreReader::ptr contents)> when_mirror_write_finishes;
	std::function<void(DropParams)>                                   when_drop_finishes;

	struct Options
	{
		bool partition_keys = true;
		bool write_chaining = true;
		bool active_sync = true;
		bool udt = true;
	} options;
};

