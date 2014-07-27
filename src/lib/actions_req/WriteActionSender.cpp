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
	std::shared_ptr<ConnectionWriteStream> conn = open(peer, write);
	if (!conn)
		return false;
	return store(*conn, write, contents);
}

std::shared_ptr<ConnectionWriteStream> WriteActionSender::open(const Peer& peer, const WriteParams& write)
{
	shared_ptr<IBufferedConnectionWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return NULL;

	// should really dereference the shared_ptr, unless we need to carry CWS elsewhere...
	// also, store peer.actionId in WriteParams? "WriteInstructions"?
	// also also, could store a IConnectionWriteStream there instead...
	// ...but that would result in keeping IBufferedConnectionWriters around.
	// OTOH, it's less crap to put in the WriteParams (one ptr)
	shared_ptr<ConnectionWriteStream> conn(new ConnectionWriteStream(writer, peer.nextActionId(), _blocking));

	std::stringstream ss;
	ss << "write|name=" << write.filename << " i=" << write.mirror << " n=" << write.totalCopies << " v=" << write.version << " offset=" << write.offset;
	if (!write.source.empty())
		ss << " source=" << write.source;
	ss << "|";
	std::string buff(ss.str());
	conn->write(buff.data(), buff.size());

	return conn;
}

bool WriteActionSender::store(ConnectionWriteStream& conn, const WriteParams& write, IDataStoreReader::ptr contents)
{
	// TODO: we can expect this read loop to fail at some point when EnsureDelivery is false.
	//  introduce PartialTransfers object (inside BufferedConnectionWriter?) to hold onto IDataStoreReader::ptrs
	//  and our transfer progress (bytesWrit)
	//

	// TODO: writer will be totally different, since we're doing:
	// WAN writes:
	//     open(), version(), getMirror()
	//     mirror_write() to next mirror
	//     dataStore_write() to local dataStore

	// need effective way to handle failed mirror_write() calls.
	//  have PartialTransfers hold send buffers?
	//  (before eventually paging to the file stored on disk?)

	// threading:
	//   local writes: disk (data store) write occurs on different thread.
	//                 network write blocks on my thread.
	//   wan writes: disk write blocks on my thread
	//               network writes are async.


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

		// here, we load PartialTransfers with the WriteParams and maybe reader. (we might make him go get it out of the DataStore...)
		std::cout << "write of " << write.filename << " blew up after " << bytesWrit << " bytes" << std::endl;
		return false;
	}

	if (write.isComplete)
	{
		conn.write(NULL, 0);
		conn.flush();
	}
	return true;
}
