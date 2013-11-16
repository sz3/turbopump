#pragma once

#include "data_store/IDataStoreReader.h"
#include <string>

struct TurboApi
{
	std::function<void(std::string filename, IDataStoreReader::ptr contents)> when_local_write_finishes;
	std::function<void(std::string filename, IDataStoreReader::ptr contents)> when_remote_write_finishes;
};

