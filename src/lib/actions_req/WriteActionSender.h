#pragma once

#include "data_store/IDataStoreReader.h"
#include <string>

class IPeerTracker;
class Peer;

class WriteActionSender
{
public:
	WriteActionSender(IPeerTracker& peers);

	bool store(const Peer& peer, const std::string& filename, IDataStoreReader::ptr contents);

protected:
	IPeerTracker& _peers;
};
