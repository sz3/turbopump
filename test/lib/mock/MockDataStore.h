/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "data_store/IDataStoreWriter.h"

#include "util/CallHistory.h"
#include <map>

class MockDataStore : public IDataStore
{
protected:
	class Writer : public IDataStoreWriter
	{
	public:
		Writer(std::string filename, MockDataStore& store);

		bool write(const char* buffer, unsigned size);
		IDataStoreReader::ptr commit();

	public:
		std::string _filename;
		std::string _buffer;
		MockDataStore& _store;
	};

	class Reader : public IDataStoreReader
	{
	public:
		Reader(const std::string& data);

		bool seek(unsigned long long offset);
		int read(IByteStream& out);

	public:
		std::string _data;
		unsigned long long _offset;
	};

protected:
	IDataStoreReader::ptr commit(Writer& writer);

public:
	std::shared_ptr<IDataStoreWriter> write(const std::string& filename);
	std::shared_ptr<IDataStoreReader> read(const std::string& filename) const;
	bool erase(const std::string& filename);

	std::string toString() const;

public:
	using data_map_type = std::map< std::string, std::string >;
	data_map_type _store;

	mutable CallHistory _history;
};

