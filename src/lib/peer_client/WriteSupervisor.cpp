/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WriteSupervisor.h"

#include "IPartialTransfers.h"
#include "api/WriteInstructions.h"
#include "membership/Peer.h"
#include "peer_client/IMessagePacker.h"
#include "peer_server/BufferedConnectionWriter.h"
#include "peer_server/ConnectionWriteStream.h"
#include "storage/IStore.h"
#include "storage/readstream.h"

#include "socket/ISocketServer.h"
#include "socket/ISocketWriter.h"
#include "socket/socket_address.h"
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

WriteSupervisor::WriteSupervisor(const IMessagePacker& packer, IPartialTransfers& transfers, ISocketServer& server, const IStore& store)
	: _packer(packer)
	, _transfers(transfers)
	, _server(server)
	, _store(store)
{
}

bool WriteSupervisor::store(const Peer& peer, const WriteInstructions& write, readstream& contents)
{
	std::shared_ptr<ConnectionWriteStream> conn = open(peer, write, true);
	if (!conn)
		return false;
	return store(*conn, write, contents);
}

std::shared_ptr<ConnectionWriteStream> WriteSupervisor::open(const Peer& peer, const WriteInstructions& write, bool blocking)
{
	socket_address addr;
	if (!addr.fromString(peer.address()))
		return NULL;

	shared_ptr<ISocketWriter> writer(_server.getWriter(addr));
	if (!writer)
		return NULL;

	shared_ptr<ConnectionWriteStream> conn(new ConnectionWriteStream(writer, blocking));

	std::string buff(reqHeader(_packer, write));
	conn->write(buff.data(), buff.size());
	return conn;
}

// only write on as WriteAction callback until we hit our initial failure.
// from that point on, the cleanup (PartialTransfers) will take over ownership of the connectionwritestream
// there is a race condition where we don't finish writing the file before partialtransfers finishes his work,
// but that should (hopefully) be rare.
// TODO: find some way to defer back to the WriteAction thread for that corner case.
bool WriteSupervisor::store(ConnectionWriteStream& conn, const WriteInstructions& write, readstream& contents, bool background)
{
	// we can expect this read loop to fail at some point when EnsureDelivery is false.
	//  when it does, transfer control shifts to the background/cleanup thread.
	//  if we're not on it, bail out.
	if (conn.background() and !background)
		return false;

	// need effective way to handle failed mirror_write() calls.
	//  have PartialTransfers hold send buffers?
	//  (before eventually paging to the file stored on disk?)

	// TODO: rewrite read loop entirely to be callback driven, in prep for disk IO which occurs on a different thread.

	// TODO: seek()/setPosition() is tied to WriteCommand setting the offset from the writer. The Writer also produces the reader. Should it happen automagically?
	// I'm thinking *not*, just to make it consistent across callbacks. No guarantee that the file position on `contents` is where you want it to be anyway, so setPosition() is the safe thing to do.
	if (write.offset > 0)
	{
		if (!contents.setPosition(write.offset))
			return false;
	}

	int bytesWrit = 0;
	int wrote = 0;
	while ((wrote = contents.stream(conn)) > 0)
		bytesWrit += wrote;

	if (wrote == -1 || conn.full())
	{
		conn.setBackground(true);

		WriteInstructions rewrite(write);
		rewrite.isComplete = true;
		rewrite.offset += bytesWrit;
		_transfers.add( conn.writer()->handle(), std::bind(&WriteSupervisor::resume, this, rewrite) );
		// schedule again
		return false;
	}

	if (write.isComplete)
	{
		conn.write(NULL, 0);
		conn.flush();
	}
	return true;
}

// perhaps rely on readstream to determine completeness rather than isComplete flag?
// that way we wouldn't have to guess...
bool WriteSupervisor::resume(const WriteInstructions& write)
{
	// read won't work if the write is in progress...
	readstream reader = _store.read(write.name, write.version, false);
	if (!reader)
		return true; // we're done.

	return store(*write.outstream, write, reader, true);
}
