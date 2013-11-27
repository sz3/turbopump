#pragma once

#include "IWriteActionSender.h"
class IPeerTracker;

class WriteActionSender : public IWriteActionSender
{
public:
	WriteActionSender(IPeerTracker& peers);

	bool store(const Peer& peer, const std::string& filename, IDataStoreReader::ptr contents);

protected:
	IPeerTracker& _peers;
};
