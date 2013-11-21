#include "WriteActionSender.h"

#include "membership/Peer.h"
#include "wan_server/IPeerTracker.h"
#include "wan_server/PeerConnection.h"
#include "wan_server/ThrottledWriteStream.h"
#include <memory>

using std::shared_ptr;
using std::string;

WriteActionSender::WriteActionSender(IPeerTracker& peers)
	: _peers(peers)
{
}

bool WriteActionSender::store(const Peer& peer, const string& filename, IDataStoreReader::ptr contents)
{
	shared_ptr<PeerConnection> peerConn = _peers.track(peer);
	if (!peerConn)
		return false;

	ThrottledWriteStream writer(*peerConn);

	string buff("write|name=" + filename + "|");
	writer.write(buff.data(), buff.size());
	contents->read(writer);
	writer.write(NULL, 0);

	return true;
}
