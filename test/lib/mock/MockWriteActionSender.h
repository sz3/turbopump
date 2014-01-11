/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "actions_req/IWriteActionSender.h"
#include "util/CallHistory.h"
class IPeerTracker;

class MockWriteActionSender : public IWriteActionSender
{
public:
	MockWriteActionSender();
	bool store(const Peer& peer, const KeyMetadata& file, IDataStoreReader::ptr contents);

public:
	CallHistory _history;
	bool _storeFails;
};

