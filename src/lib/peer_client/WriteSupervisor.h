/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ISuperviseWrites.h"
class IPeerTracker;

class WriteSupervisor : public ISuperviseWrites
{
public:
	WriteSupervisor(IPeerTracker& peers);

	bool store(const Peer& peer, const WriteInstructions& write, IDataStoreReader::ptr contents);

	std::shared_ptr<ConnectionWriteStream> open(const Peer& peer, const WriteInstructions& write, bool blocking);
	bool store(ConnectionWriteStream& conn, const WriteInstructions& write, IDataStoreReader::ptr contents);

protected:
	IPeerTracker& _peers;
};
