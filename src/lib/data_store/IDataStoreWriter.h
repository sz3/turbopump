/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class IDataStoreReader;
class KeyMetadata;

class IDataStoreWriter
{
public:
	virtual ~IDataStoreWriter() {}

	typedef std::shared_ptr<IDataStoreWriter> ptr;

	virtual bool write(const char* buffer, unsigned size) = 0;
	virtual std::shared_ptr<IDataStoreReader> commit() = 0;

	virtual KeyMetadata& metadata() = 0;
};
