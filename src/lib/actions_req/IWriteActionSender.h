#pragma once

#include "data_store/IDataStoreReader.h"
#include <string>
class Peer;

class IWriteActionSender
{
public:
	virtual ~IWriteActionSender() {}

	virtual bool store(const Peer& peer, const std::string& filename, IDataStoreReader::ptr contents) = 0;
};
