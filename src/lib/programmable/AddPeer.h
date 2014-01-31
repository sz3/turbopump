/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"

class IHashRing;
class IMembership;
class IKeyTabulator;
class KeyMetadata;

class AddPeer
{
public:
	AddPeer(IHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator);

	bool run(KeyMetadata md, IDataStoreReader::ptr contents);

protected:
	IHashRing& _ring;
	IMembership& _membership;
	IKeyTabulator& _keyTabulator;
};
