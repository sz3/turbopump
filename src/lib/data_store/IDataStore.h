/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
// async class to do reads/writes. May be local, may be remote.
// we CAN do work on the caller thread, but only if it's fast (e.g. no IO/network/mutexes)

#include <functional>
#include <memory>
#include <string>

class IByteStream;
class IDataStoreReader;
class IDataStoreWriter;

class IDataStore
{
public:
	virtual ~IDataStore() {}

	virtual std::shared_ptr<IDataStoreWriter> write(const std::string& filename) = 0;
	virtual std::shared_ptr<IDataStoreWriter> write(const std::string& filename, const std::string& version) = 0;
	virtual std::shared_ptr<IDataStoreReader> read(const std::string& filename) const = 0;
	virtual bool erase(const std::string& filename) = 0;

	virtual void report(IByteStream& writer, const std::string& exclude="") const = 0;
};

