/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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
