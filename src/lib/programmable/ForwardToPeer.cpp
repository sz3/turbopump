#include "ForwardToPeer.h"

#include "actions_req/WriteActionSender.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <memory>
using std::shared_ptr;

ForwardToPeer::ForwardToPeer(const IMembership& membership, IPeerTracker& peers)
	: _membership(membership)
	, _peers(peers)
{
}

bool ForwardToPeer::run(std::string filename, IDataStoreReader::ptr contents)
{
	shared_ptr<Peer> peer = _membership.randomPeer();
	if (!peer)
		return false;

	WriteActionSender client(_peers);
	return client.store(*peer, filename, contents);
}
