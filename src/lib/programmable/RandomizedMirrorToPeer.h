#pragma once

#include "data_store/IDataStoreReader.h"

class IMembership;
class IPeerTracker;
class KeyMetadata;

class RandomizedMirrorToPeer
{
public:
	RandomizedMirrorToPeer(const IMembership& membership, IPeerTracker& peers);

	bool run(KeyMetadata md, IDataStoreReader::ptr contents);

protected:
	const IMembership& _membership;
	IPeerTracker& _peers;
};
