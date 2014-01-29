/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ISynchronize.h"
#include <string>
class ICorrectSkew;
class IHashRing;
class IMembership;
class IKeyTabulator;
class IMessageSender;

class Synchronizer : public ISynchronize
{
public:
	Synchronizer(const IHashRing& ring, const IMembership& membership, const IKeyTabulator& index, IMessageSender& messenger, ICorrectSkew& corrector);

	void pingRandomPeer();
	void offloadUnwantedKeys();
	void compare(const Peer& peer, const TreeId& treeid, const MerklePoint& point);
	void pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last);

protected:
	const IHashRing& _ring;
	const IMembership& _membership;
	const IKeyTabulator& _index;
	IMessageSender& _messenger;
	ICorrectSkew& _corrector;
};
