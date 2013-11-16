#include "ForwardToPeer.h"

#include "membership/IMembership.h"
#include "membership/Peer.h"
#include "wan_server/IPeerTracker.h"
#include "wan_server/PeerConnection.h"
#include "wan_server/ThrottledWriteStream.h"
#include <memory>

using std::shared_ptr;
using std::string;

ForwardToPeer::ForwardToPeer(const IMembership& membership, IPeerTracker& peers)
	: _membership(membership)
	, _peers(peers)
{
}

bool ForwardToPeer::run(std::string filename, IDataStoreReader::ptr contents)
{
	shared_ptr<Peer> peer;
	for (int i = 0; i < 3; ++i)
	{
		peer = _membership.randomPeer();
		if (peer && peer != _membership.self())
			break;
	}
	if (!peer)
		return false;

	shared_ptr<PeerConnection> peerConn = _peers.track(*peer);
	if (!peerConn)
		return false;

	ThrottledWriteStream writer(*peerConn);

	string buff("write|name=" + filename + "|");
	writer.write(buff.data(), buff.size());
	contents->read(writer);
	writer.write(NULL, 0);

	return true;
}
