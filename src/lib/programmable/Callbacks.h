/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "TurboApi.h"

class IHashRing;
class IMembership;
class IMerkleIndex;
class IPeerTracker;

// not really sure about the name of this guy, yet
class Callbacks : public TurboApi
{
public:
	Callbacks();
	Callbacks(const TurboApi& instruct);

	void initialize(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers, IMerkleIndex& merkleIndex);

protected:
};

