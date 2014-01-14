/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class DataEntry;
class IByteStream;

class IDataStoreReader
{
public:
	virtual ~IDataStoreReader() {}

	typedef std::shared_ptr<IDataStoreReader> ptr;

	virtual bool seek(unsigned long long offset) = 0;
	virtual int read(IByteStream& out) = 0;

	// this exists to expose access to the metadata... but there may be a better way to go about it
	virtual const DataEntry& data() const = 0;
};
