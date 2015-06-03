/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ISynchronize.h"
#include <memory>
#include <string>
#include <vector>
class ICorrectSkew;
class IConsistentHashRing;
class IKnowPeers;
class IKeyTabulator;
class ILog;
class IMessageSender;

class Synchronizer : public ISynchronize
{
public:
	Synchronizer(const IConsistentHashRing& ring, const IKnowPeers& membership, const IKeyTabulator& index, IMessageSender& messenger, ICorrectSkew& corrector, ILog& logger);

	void pingRandomPeer();
	void offloadUnwantedKeys();
	void compare(const Peer& peer, const TreeId& treeid, const MerklePoint& point, bool isSyncResponse=false);

protected:
	std::shared_ptr<Peer> randomPeerFromList(std::vector<std::string> locs) const;

protected:
	const IConsistentHashRing& _ring;
	const IKnowPeers& _membership;
	const IKeyTabulator& _index;
	IMessageSender& _messenger;
	ICorrectSkew& _corrector;
	ILog& _logger;
};
