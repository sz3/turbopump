/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"

class IHashRing;
class IMembership;
class IMerkleIndex;
class KeyMetadata;

class AddPeer
{
public:
	AddPeer(IHashRing& ring, IMembership& membership, IMerkleIndex& merkleIndex);

	bool run(KeyMetadata md, IDataStoreReader::ptr contents);

protected:
	IHashRing& _ring;
	IMembership& _membership;
	IMerkleIndex& _merkleIndex;
};
