/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IDataStore.h"
#include "IDataStoreReader.h"
#include "IDataStoreWriter.h"
// local RAM.

#include "tbb/concurrent_unordered_map.h"

class LocalDataStore : public IDataStore
{
protected:
	class Writer : public IDataStoreWriter
	{
	public:
		Writer(std::string filename, LocalDataStore& store);

		bool write(const char* buffer, unsigned size);
		IDataStoreReader::ptr commit();

		std::string&& filename();
		std::string&& buffer();

	protected:
		std::string _filename;
		std::string _buffer;
		LocalDataStore& _store;
	};

	class Reader : public IDataStoreReader
	{
	public:
		Reader(const std::shared_ptr<std::string>& data);

		bool seek(unsigned long long offset);
		int read(IByteStream& out);

	protected:
		std::shared_ptr<std::string> _data;
		unsigned long long _offset;
	};
public:
	std::shared_ptr<IDataStoreWriter> write(const std::string& filename);
	std::shared_ptr<IDataStoreReader> read(const std::string& filename) const;
	bool erase(const std::string& filename);

	std::string toString() const;

protected:
	IDataStoreReader::ptr commit(Writer& writer);

protected:
	using data_map_type = tbb::concurrent_unordered_map< std::string, std::shared_ptr<std::string> >;
	data_map_type _store;
};

