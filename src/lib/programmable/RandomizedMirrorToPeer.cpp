#include "RandomizedMirrorToPeer.h"

#include "actions_req/WriteActionSender.h"
#include "common/KeyMetadata.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <memory>
using std::shared_ptr;

RandomizedMirrorToPeer::RandomizedMirrorToPeer(const IMembership& membership, IPeerTracker& peers)
	: _membership(membership)
	, _peers(peers)
{
}

bool RandomizedMirrorToPeer::run(KeyMetadata md, IDataStoreReader::ptr contents)
{
	shared_ptr<Peer> peer = _membership.randomPeer();
	if (!peer)
		return false;

	WriteActionSender client(_peers);
	return client.store(*peer, md.filename, contents);
}
