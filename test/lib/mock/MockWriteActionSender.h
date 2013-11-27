#pragma once

#include "actions_req/IWriteActionSender.h"
#include "util/CallHistory.h"
class IPeerTracker;

class MockWriteActionSender : public IWriteActionSender
{
public:
	MockWriteActionSender();
	bool store(const Peer& peer, const std::string& filename, IDataStoreReader::ptr contents);

public:
	CallHistory _history;
	bool _storeFails;
};

