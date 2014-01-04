#include "MirrorToPeer.h"

#include "actions_req/WriteActionSender.h"
#include "common/KeyMetadata.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include <memory>
using std::shared_ptr;

MirrorToPeer::MirrorToPeer(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers)
	: _ring(ring)
	, _membership(membership)
	, _peers(peers)
{
}

// need to tell me what mirror number I'm supposed to be. e.g. 0, 1, 2, 3. Also need number of desired copies. (0 == infinite?)
// TODO: specify number of copies -- 1,2,3,5
// TODO: specify consistency?
bool MirrorToPeer::run(KeyMetadata md, IDataStoreReader::ptr contents)
{
	std::vector<std::string> locations = _ring.lookup(md.filename, 5);
	// do something with locations

	shared_ptr<Peer> peer = _membership.randomPeer();
	if (!peer)
		return false;

	WriteActionSender client(_peers);
	return client.store(*peer, md.filename, contents);
}
