/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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
