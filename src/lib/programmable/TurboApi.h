/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"
class KeyMetadata;

struct TurboApi
{
	std::function<void(KeyMetadata md, IDataStoreReader::ptr contents)> when_local_write_finishes;
	std::function<void(KeyMetadata md, IDataStoreReader::ptr contents)> when_mirror_write_finishes;
	std::function<void(KeyMetadata md)>                                 when_drop_finishes;

	struct Options
	{
		bool partition_keys = true;
		bool write_chaining = true;
		bool merkle = true;
		bool udt = true;
	} options;
};

