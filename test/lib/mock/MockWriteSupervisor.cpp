/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockWriteSupervisor.h"

#include "actions/WriteParams.h"
#include "membership/Peer.h"
#include "serialize/StringUtil.h"
#include "wan_server/ConnectionWriteStream.h"

MockWriteSupervisor::MockWriteSupervisor()
	: _storeFails(false)
{
}

bool MockWriteSupervisor::store(const Peer& peer, const WriteParams& write, IDataStoreReader::ptr contents)
{
	_history.call("store", peer.uid, write.filename, write.mirror, write.totalCopies, "["+write.version+"]", write.source, write.isComplete);
	return !_storeFails;
}

std::shared_ptr<ConnectionWriteStream> MockWriteSupervisor::open(const Peer& peer, const WriteParams& write, bool blocking)
{
	_history.call("open", peer.uid, write.filename, blocking);
	if (!_writer)
		return NULL;
	return std::shared_ptr<ConnectionWriteStream>( new ConnectionWriteStream(_writer, 1, blocking) );
}

bool MockWriteSupervisor::store(ConnectionWriteStream& conn, const WriteParams& write, IDataStoreReader::ptr contents)
{
	_history.call("store", write.filename + "|" + write.version + "|" + StringUtil::str(write.isComplete), contents->size());
	return true;
}
