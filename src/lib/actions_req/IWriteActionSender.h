/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"
class KeyMetadata;
class Peer;

class IWriteActionSender
{
public:
	virtual ~IWriteActionSender() {}

	virtual bool store(const Peer& peer, const KeyMetadata& file, IDataStoreReader::ptr contents) = 0;
};
