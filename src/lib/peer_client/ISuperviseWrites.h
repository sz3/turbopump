/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class ConnectionWriteStream;
class Peer;
class WriteInstructions;
class readstream;

class ISuperviseWrites
{
public:
	virtual ~ISuperviseWrites() {}

	virtual bool store(const Peer& peer, const WriteInstructions& write, readstream& contents) = 0;

	virtual std::shared_ptr<ConnectionWriteStream> open(const Peer& peer, const WriteInstructions& write, bool blocking) = 0;
	virtual bool store(ConnectionWriteStream& conn, const WriteInstructions& write, readstream& contents) = 0;
};
