#pragma once

#include "data_store/IDataStoreReader.h"

class IHashRing;
class IMembership;
class IPeerTracker;
class KeyMetadata;

class MirrorToPeer
{
public:
	MirrorToPeer(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers);

	bool run(KeyMetadata md, IDataStoreReader::ptr contents);

protected:
	const IHashRing& _ring;
	const IMembership& _membership;
	IPeerTracker& _peers;
};
