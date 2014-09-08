/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "actions_req/ISuperviseWrites.h"
#include "util/CallHistory.h"
class IBufferedConnectionWriter;
class IPeerTracker;

class MockWriteSupervisor : public ISuperviseWrites
{
public:
	MockWriteSupervisor();
	bool store(const Peer& peer, const WriteParams& write, IDataStoreReader::ptr contents);

	std::shared_ptr<ConnectionWriteStream> open(const Peer& peer, const WriteParams& write, bool blocking);
	bool store(ConnectionWriteStream& conn, const WriteParams& write, IDataStoreReader::ptr contents);

public:
	CallHistory _history;
	std::shared_ptr<IBufferedConnectionWriter> _writer;
	bool _storeFails;
};
