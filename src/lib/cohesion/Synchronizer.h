#pragma once

#include "ISynchronize.h"
#include <string>
class ICorrectSkew;
class IMembership;
class IMerkleIndex;
class IPeerTracker;

class Synchronizer : public ISynchronize
{
public:
	Synchronizer(const IMembership& membership, IPeerTracker& peers, const IMerkleIndex& index, ICorrectSkew& corrector);

	void pingRandomPeer();
	void compare(const Peer& peer, const MerklePoint& point);

protected:
	bool sendMessage(const Peer& peer, const std::string& message);

protected:
	const IMembership& _membership;
	IPeerTracker& _peers;
	const IMerkleIndex& _index;
	ICorrectSkew& _corrector;
};
