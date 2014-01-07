/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ISynchronize.h"
#include <string>
class ICorrectSkew;
class IMembership;
class IMerkleIndex;
class IMessageSender;

// this probably should be the "high level" interface overseeing synchronization.
// that means it probably shouldn't be sending packets itself, but rather delegating to pieces that do?
class Synchronizer : public ISynchronize
{
public:
	Synchronizer(const IMembership& membership, const IMerkleIndex& index, IMessageSender& messenger, ICorrectSkew& corrector);

	void pingRandomPeer();
	void compare(const Peer& peer, const MerklePoint& point);
	void pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last);

protected:
	const IMembership& _membership;
	const IMerkleIndex& _index;
	IMessageSender& _messenger;
	ICorrectSkew& _corrector;
};
