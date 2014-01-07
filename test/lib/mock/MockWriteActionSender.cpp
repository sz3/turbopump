/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockWriteActionSender.h"

#include "membership/Peer.h"

MockWriteActionSender::MockWriteActionSender()
	: _storeFails(false)
{
}

bool MockWriteActionSender::store(const Peer& peer, const std::string& filename, IDataStoreReader::ptr contents)
{
	_history.call("store", peer.uid, filename);
	return !_storeFails;
}
