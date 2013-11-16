#pragma once

#include <memory>
class IByteStream;

class IDataStoreReader
{
public:
	virtual ~IDataStoreReader() {}

	typedef std::shared_ptr<IDataStoreReader> ptr;

	virtual bool seek(unsigned long long offset) = 0;
	virtual int read(IByteStream& out) = 0;
};
