#pragma once
// async class to do reads/writes. May be local, may be remote.
// we CAN do work on the caller thread, but only if it's fast (e.g. no IO/network/mutexes)

#include <functional>
#include <memory>
#include <string>

class IDataStoreReader;
class IDataStoreWriter;

class IDataStore
{
public:
	virtual ~IDataStore() {}

	virtual std::shared_ptr<IDataStoreWriter> write(const std::string& filename) = 0;
	virtual std::shared_ptr<IDataStoreReader> read(const std::string& filename) = 0;
	virtual void erase(const std::string& filename, const std::function<void(bool)>& callback) = 0;

	// virtual IDataStoreIterator enumerate(unsigned long long hash) const;
	// virtual IDataStoreIterator end() const;
};

