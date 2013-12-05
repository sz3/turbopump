#include "WriteActionSender.h"

#include "membership/Peer.h"
#include "wan_server/BufferedSocketWriter.h"
#include "wan_server/IPeerTracker.h"
#include <memory>
using std::string;
using std::unique_ptr;

WriteActionSender::WriteActionSender(IPeerTracker& peers)
	: _peers(peers)
{
}

bool WriteActionSender::store(const Peer& peer, const string& filename, IDataStoreReader::ptr contents)
{
	unique_ptr<BufferedSocketWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return false;

	string buff("write|name=" + filename + "|");
	writer->write(buff.data(), buff.size());
	contents->read(*writer);
	writer->write(NULL, 0);

	return true;
}
