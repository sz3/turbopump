/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ISynchronize.h"
#include <string>
class ICorrectSkew;
class IConsistentHashRing;
class IMembership;
class IKeyTabulator;
class ILog;
class IMessageSender;

class Synchronizer : public ISynchronize
{
public:
	Synchronizer(const IConsistentHashRing& ring, const IMembership& membership, const IKeyTabulator& index, IMessageSender& messenger, ICorrectSkew& corrector, ILog& logger);

	void pingRandomPeer();
	void offloadUnwantedKeys();
	void compare(const Peer& peer, const TreeId& treeid, const MerklePoint& point, bool isSyncResponse=false);

protected:
	const IConsistentHashRing& _ring;
	const IMembership& _membership;
	const IKeyTabulator& _index;
	IMessageSender& _messenger;
	ICorrectSkew& _corrector;
	ILog& _logger;
};
