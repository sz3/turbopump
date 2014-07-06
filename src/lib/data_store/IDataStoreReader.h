/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class IByteStream;
class KeyMetadata;

class IDataStoreReader
{
public:
	virtual ~IDataStoreReader() {}

	typedef std::shared_ptr<IDataStoreReader> ptr;

	virtual bool seek(unsigned long long offset) = 0;
	virtual int read(IByteStream& out) = 0;

	virtual const KeyMetadata& metadata() const = 0;
	virtual unsigned long long size() const = 0;
	virtual unsigned long long summary() const = 0;
};
