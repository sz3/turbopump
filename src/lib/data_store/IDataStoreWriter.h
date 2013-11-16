#pragma once

#include <memory>

class IDataStoreReader;

class IDataStoreWriter
{
public:
	virtual ~IDataStoreWriter() {}

	typedef std::shared_ptr<IDataStoreWriter> ptr;

	virtual bool write(const char* buffer, unsigned size) = 0;
	virtual std::shared_ptr<IDataStoreReader> commit() = 0;
};
