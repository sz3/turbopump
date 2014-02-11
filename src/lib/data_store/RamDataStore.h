/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "DataEntry.h"
#include "IDataStore.h"
#include "IDataStoreReader.h"
#include "IDataStoreWriter.h"
// local RAM.

#include "tbb/concurrent_unordered_map.h"

class RamDataStore : public IDataStore
{
protected:
	class Writer : public IDataStoreWriter
	{
	public:
		Writer(std::string filename, RamDataStore& store);
		KeyMetadata& metadata();

		bool write(const char* buffer, unsigned size);
		IDataStoreReader::ptr commit();

		std::string&& move_filename();
		DataEntry&& move_data();

	protected:
		std::string _filename;
		DataEntry _data;
		RamDataStore& _store;
	};

	class Reader : public IDataStoreReader
	{
	public:
		Reader(const std::shared_ptr<DataEntry>& data);
		const KeyMetadata& metadata() const;

		bool seek(unsigned long long offset);
		int read(IByteStream& out);

	protected:
		std::shared_ptr<DataEntry> _data;
		unsigned long long _offset;
	};

public:
	std::shared_ptr<IDataStoreWriter> write(const std::string& filename);
	std::shared_ptr<IDataStoreWriter> write(const std::string& filename, const std::string& version);
	std::shared_ptr<IDataStoreReader> read(const std::string& filename) const;
	bool erase(const std::string& filename);

	void report(IByteStream& writer, const std::string& exclude="") const;

protected:
	IDataStoreReader::ptr commit(Writer& writer);

protected:
	using data_map_type = tbb::concurrent_unordered_map< std::string, std::shared_ptr<DataEntry> >;
	data_map_type _store;
};

