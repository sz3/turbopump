/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"

class IMembership;
class IPeerTracker;
class WriteParams;

class RandomizedMirrorToPeer
{
public:
	RandomizedMirrorToPeer(const IMembership& membership, IPeerTracker& peers);

	bool run(WriteParams params, IDataStoreReader::ptr contents);

protected:
	const IMembership& _membership;
	IPeerTracker& _peers;
};
