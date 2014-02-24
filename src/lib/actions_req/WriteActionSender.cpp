/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WriteActionSender.h"

#include "actions/WriteParams.h"
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

bool WriteActionSender::store(const Peer& peer, const WriteParams& write, IDataStoreReader::ptr contents)
{
	shared_ptr<IBufferedConnectionWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return false;

	ConnectionWriteStream stream(writer, peer.nextActionId());

	std::stringstream ss;
	ss << "write|name=" << write.filename << " i=" << write.mirror << " n=" << write.totalCopies << " v=" << write.version;
	if (!write.source.empty())
		ss << " source=" << write.source;
	ss << "|";
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
