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
