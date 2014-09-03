/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStoreReader.h"
class ConnectionWriteStream;
class Peer;
class WriteParams;

class ISuperviseWrites
{
public:
	virtual ~ISuperviseWrites() {}

	virtual bool store(const Peer& peer, const WriteParams& write, IDataStoreReader::ptr contents) = 0;

	virtual std::shared_ptr<ConnectionWriteStream> open(const Peer& peer, const WriteParams& write, bool blocking) = 0;
	virtual bool store(ConnectionWriteStream& conn, const WriteParams& write, IDataStoreReader::ptr contents) = 0;
};
