/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "RamDataStore.h"

#include "IDataStoreWriter.h"
#include "serialize/StringUtil.h"
#include "socket/IByteStream.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <sstream>
using namespace std::placeholders;
using std::bind;
using std::shared_ptr;
using std::string;

std::shared_ptr<IDataStoreWriter> RamDataStore::write(const string& filename)
{
	return IDataStoreWriter::ptr(new Writer(filename, *this));
}

/*
  ******************************************
  R a m  D a t a  S t o r e  ::  W r i t e r
  ******************************************
*/
RamDataStore::Writer::Writer(std::string filename, RamDataStore& store)
	: _filename(std::move(filename))
	, _offset(0)
	, _store(store)
{
}

bool RamDataStore::Writer::write(const char* buffer, unsigned size)
{
	_data.data.append(buffer, size);
	return true;
}

IDataStoreReader::ptr RamDataStore::Writer::commit()
{
	return _store.commit(*this);
}

std::string&& RamDataStore::Writer::move_filename()
{
	return std::move(_filename);
}

DataEntry&& RamDataStore::Writer::move_data()
{
	return std::move(_data);
}

void RamDataStore::Writer::setOffset(unsigned long long offset)
{
	_offset = offset;
}

unsigned long long RamDataStore::Writer::offset() const
{
	return _offset;
}

KeyMetadata& RamDataStore::Writer::metadata()
{
	return _data.md;
}
/*
  </end child class>
*/

// version on commit?
IDataStoreReader::ptr RamDataStore::commit(Writer& writer)
{
	DataChain& chain = _store[writer.move_filename()];
	shared_ptr<DataEntry> data(new DataEntry(writer.move_data()));
	if (data->md.version.empty())
		chain.storeAsBestVersion(data);
	else
		chain.store(data, writer.offset());
	return IDataStoreReader::ptr(new Reader(data, chain.summary()));
}

std::vector< shared_ptr<IDataStoreReader> > RamDataStore::read(const string& filename) const
{
	std::vector<IDataStoreReader::ptr> reads;

	data_map_type::const_iterator it = _store.find(filename);
	if (it == _store.end())
		return reads;

	std::vector< shared_ptr<DataEntry> > entries = it->second.entries();
	for (auto entry = entries.begin(); entry != entries.end(); ++entry)
	{
		if ( !(*entry)->md.version.isDeleted() )
			reads.push_back(IDataStoreReader::ptr(new Reader(*entry)));
	}
	return reads;
}

shared_ptr<IDataStoreReader> RamDataStore::read(const string& filename, const string& version) const
{
	data_map_type::const_iterator it = _store.find(filename);
	if (it == _store.end())
		return NULL;

	VectorClock versionClock;
	if (!versionClock.fromString(version))
		return NULL;

	shared_ptr<DataEntry> data = it->second.find(versionClock);
	if (!data)
		return NULL;
	return IDataStoreReader::ptr( new Reader(data) );
}

/*
  **********************************************
  R a m  D a t a  S t o r e  ::  R e a d e r
  **********************************************
*/

RamDataStore::Reader::Reader(const std::shared_ptr<DataEntry>& data, unsigned long long summary)
	: _data(data)
	, _summary(summary)
	, _offset(0)
{
}

bool RamDataStore::Reader::seek(unsigned long long offset)
{
	if (offset > _data->data.size())
		return false;
	_offset = offset;
	return true;
}

int RamDataStore::Reader::read(IByteStream& out)
{
	long long numBytes = _data->data.size() - _offset;
	if (numBytes <= 0)
		return 0;

	if (out.maxPacketLength() < numBytes)
		numBytes = out.maxPacketLength();

	const char* start = &(_data->data)[_offset];
	_offset += numBytes;
	return out.write(start, numBytes);
}

const KeyMetadata& RamDataStore::Reader::metadata() const
{
	return _data->md;
}

unsigned long long RamDataStore::Reader::summary() const
{
	return _summary;
}
/*
  </end child class>
*/

// drop all knowledge of a key.
// generally only used after offloading a key to another box.
bool RamDataStore::drop(const string& filename)
{
	// TODO: reader/writer locks.
	// this is the only "writer"
	return _store.unsafe_erase(filename) > 0;
}

void RamDataStore::report(IByteStream& writer, bool showDeleted, const string& exclude) const
{
	bool first = true;
	for (data_map_type::const_iterator it = _store.begin(); it != _store.end(); ++it)
	{
		if (!exclude.empty() && it->first.find(exclude) != string::npos)
			continue;

		std::vector< shared_ptr<DataEntry> > entries = it->second.entries();
		string fileReport;
		for (auto entry = entries.begin(); entry != entries.end(); ++entry)
		{
			const VectorClock& version = (*entry)->md.version;
			if (!showDeleted && version.isDeleted())
				continue;
			if (entry != entries.begin())
				fileReport += " ";
			fileReport += StringUtil::str((*entry)->data.size());
			fileReport += "|" + version.toString();
		}
		if (fileReport.empty())
			continue;

		fileReport = "(" + it->first + ")=>" + fileReport;
		if (!first)
			fileReport = "\n" + fileReport;
		first &= false;
		writer.write(fileReport.data(), fileReport.size());
	}
}
