/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMessageSender.h"
class IMessagePacker;
class ISocketServer;

class MessageSender : public IMessageSender
{
public:
	MessageSender(const IMessagePacker& packer, ISocketServer& server);

	// used by Synchronizer
	void digestPing(const Peer& peer, const TreeId& treeid, const MerklePoint& point);
	void digestPing(const Peer& peer, const TreeId& treeid, const std::deque<MerklePoint>& points);
	void requestKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last);
	void requestKey(const Peer& peer, const TreeId& treeid, unsigned long long key);

	// used by actions, SkewCorrector, Callbacks, etc
	void offerWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source);
	void demandWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source);
	void acknowledgeWrite(const Peer& peer, const std::string& filename, const std::string& version, unsigned long long size);

protected:
	bool sendMessage(const Peer& peer, const std::string& message, bool blocking=false);

protected:
	const IMessagePacker& _packer;
	ISocketServer& _server;
};
