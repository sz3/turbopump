/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WriteSupervisor.h"

#include "api/WriteInstructions.h"
#include "membership/Peer.h"
#include "peer_client/IMessagePacker.h"
#include "peer_server/BufferedConnectionWriter.h"
#include "peer_server/ConnectionWriteStream.h"
#include "peer_server/IPeerTracker.h"
#include <iostream>
#include <memory>
using std::string;
using std::shared_ptr;

namespace {
	string reqHeader(const IMessagePacker& packer, const Turbopump::Write& request)
	{
		msgpack::sbuffer sbuf;
		msgpack::pack(&sbuf, request);
		return packer.package(request._INTERNAL_ID, sbuf.data(), sbuf.size());
	}
}

WriteSupervisor::WriteSupervisor(const IMessagePacker& packer, IPeerTracker& peers)
	: _packer(packer)
	, _peers(peers)
{
}

bool WriteSupervisor::store(const Peer& peer, const WriteInstructions& write, IDataStoreReader::ptr contents)
{
	std::shared_ptr<ConnectionWriteStream> conn = open(peer, write, true);
	if (!conn)
		return false;
	return store(*conn, write, contents);
}

std::shared_ptr<ConnectionWriteStream> WriteSupervisor::open(const Peer& peer, const WriteInstructions& write, bool blocking)
{
	shared_ptr<IBufferedConnectionWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return NULL;

	shared_ptr<ConnectionWriteStream> conn(new ConnectionWriteStream(writer, peer.nextActionId(), blocking));

	std::string buff(reqHeader(_packer, write));
	conn->write(buff.data(), buff.size());
	return conn;
}

bool WriteSupervisor::store(ConnectionWriteStream& conn, const WriteInstructions& write, IDataStoreReader::ptr contents)
{
	// TODO: we can expect this read loop to fail at some point when EnsureDelivery is false.
	//  introduce PartialTransfers object (inside BufferedConnectionWriter?) to hold onto IDataStoreReader::ptrs
	//  and our transfer progress (bytesWrit)
	//

	// need effective way to handle failed mirror_write() calls.
	//  have PartialTransfers hold send buffers?
	//  (before eventually paging to the file stored on disk?)

	// TODO: rewrite read loop entirely to be callback driven, in prep for disk IO which occurs on a different thread.

	// TODO: some question as to whether we should always seek or not...
	if (write.offset > 0)
	{
		if (!contents->seek(write.offset))
			return false;
	}

	int bytesWrit = 0;
	int wrote = 0;
	while ((wrote = contents->read(conn)) > 0)
		bytesWrit += wrote;

	if (wrote == -1)
	{
		// TODO: event trigger on socket underneath BufferedConnectionWriter. Only is triggered if work is pending...
		//   when trigger goes, we look in PartialTransfers for work to do involving said peer.

		// here, we load PartialTransfers with the WriteInstructions and maybe reader. (we might make him go get it out of the DataStore...)
		std::cout << "write of " << write.name << " blew up after " << bytesWrit << " bytes" << std::endl;
		return false;
	}

	if (write.isComplete)
	{
		conn.write(NULL, 0);
		conn.flush();
	}
	return true;
}
