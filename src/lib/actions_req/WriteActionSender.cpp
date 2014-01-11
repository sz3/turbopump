/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WriteActionSender.h"

#include "common/KeyMetadata.h"
#include "membership/Peer.h"
#include "wan_server/BufferedConnectionWriter.h"
#include "wan_server/ConnectionWriteStream.h"
#include "wan_server/IPeerTracker.h"
#include <memory>
#include <sstream>
using std::string;
using std::shared_ptr;

WriteActionSender::WriteActionSender(IPeerTracker& peers)
	: _peers(peers)
{
}

bool WriteActionSender::store(const Peer& peer, const KeyMetadata& file, IDataStoreReader::ptr contents)
{
	shared_ptr<IBufferedConnectionWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return false;

	ConnectionWriteStream stream(writer, peer.nextActionId());

	std::stringstream ss;
	ss << "write|name=" << file.filename << " i=" << file.mirror << " n=" << file.totalCopies << "|";
	std::string buff(ss.str());
	stream.write(buff.data(), buff.size());

	int lastSuccess = 0;
	int written = 0;
	while ((written = contents->read(stream)) > 0)
		lastSuccess = written;

	//if (lastSuccess == stream.maxPacketLength())
	stream.write(NULL, 0);
	stream.flush();
	return true;
}
