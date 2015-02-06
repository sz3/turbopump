/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ISuperviseWrites.h"
class IMessagePacker;
class IPartialTransfers;
class ISocketServer;
class IStore;

class WriteSupervisor : public ISuperviseWrites
{
public:
	WriteSupervisor(const IMessagePacker& packer, IPartialTransfers& transfers, ISocketServer& server, const IStore& store);

	bool store(const Peer& peer, const WriteInstructions& write, readstream& contents);

	std::shared_ptr<ConnectionWriteStream> open(const Peer& peer, const WriteInstructions& write, bool blocking);
	bool store(ConnectionWriteStream& conn, const WriteInstructions& write, readstream& contents, bool background=false);

protected:
	bool resume(const WriteInstructions& write);

protected:
	const IMessagePacker& _packer;
	IPartialTransfers& _transfers;
	ISocketServer& _server;
	const IStore& _store;
};
