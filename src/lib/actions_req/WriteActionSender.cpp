/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WriteActionSender.h"

#include "actions/WriteParams.h"
#include "membership/Peer.h"
#include "wan_server/BufferedConnectionWriter.h"
#include "wan_server/ConnectionWriteStream.h"
#include "wan_server/IPeerTracker.h"
#include <iostream>
#include <memory>
#include <sstream>
using std::string;
using std::shared_ptr;

WriteActionSender::WriteActionSender(IPeerTracker& peers, bool blocking)
	: _peers(peers)
	, _blocking(blocking)
{
}

bool WriteActionSender::store(const Peer& peer, const WriteParams& write, IDataStoreReader::ptr contents)
{
	shared_ptr<IBufferedConnectionWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return false;

	ConnectionWriteStream stream(writer, peer.nextActionId(), _blocking);

	std::stringstream ss;
	ss << "write|name=" << write.filename << " i=" << write.mirror << " n=" << write.totalCopies << " v=" << write.version;
	if (!write.source.empty())
		ss << " source=" << write.source;
	ss << "|";
	std::string buff(ss.str());
	stream.write(buff.data(), buff.size());

	// TODO: we can expect this read loop to fail at some point when EnsureDelivery is false.
	//  introduce PartialTransfers object (inside BufferedConnectionWriter?) to hold onto IDataStoreReader::ptrs
	//  and our transfer progress (bytesWrit)

	// TODO: rewrite read loop entirely to be callback driven, in prep for disk IO which occurs on a different thread.

	int bytesWrit = 0;
	int wrote = 0;
	while ((wrote = contents->read(stream)) > 0)
		bytesWrit += wrote;

	if (wrote == -1)
	{
		// TODO: push into PartialTransfers
		std::cout << "write of " << write.filename << " to peer " << peer.uid << " blew up after " << bytesWrit << " bytes" << std::endl;
		return false;
	}

	stream.write(NULL, 0);
	stream.flush();
	return true;
}
