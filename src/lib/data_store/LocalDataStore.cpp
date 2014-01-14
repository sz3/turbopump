/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "LocalDataStore.h"

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

std::shared_ptr<IDataStoreWriter> LocalDataStore::write(const string& filename)
{
	return IDataStoreWriter::ptr(new Writer(filename, *this));
}

/*
  ******************************************
  L o c a l  D a t a  S t o r e  ::  W r i t e r
  ******************************************
*/
LocalDataStore::Writer::Writer(std::string filename, LocalDataStore& store)
	: _filename(std::move(filename))
	, _store(store)
{
}

bool LocalDataStore::Writer::write(const char* buffer, unsigned size)
{
	_data.data.append(buffer, size);
	return true;
}

IDataStoreReader::ptr LocalDataStore::Writer::commit()
{
	return _store.commit(*this);
}

std::string&& LocalDataStore::Writer::move_filename()
{
	return std::move(_filename);
}

DataEntry&& LocalDataStore::Writer::move_data()
{
	return std::move(_data);
}

DataEntry& LocalDataStore::Writer::data()
{
	return _data;
}
/*
  </end child class>
*/

IDataStoreReader::ptr LocalDataStore::commit(Writer& writer)
{
	shared_ptr<DataEntry>& data = _store[writer.move_filename()];
	data.reset(new DataEntry(writer.move_data()));
	return IDataStoreReader::ptr(new Reader(data));
}

shared_ptr<IDataStoreReader> LocalDataStore::read(const string& filename) const
{
	data_map_type::const_iterator it = _store.find(filename);
	if (it == _store.end())
		return NULL;

	return IDataStoreReader::ptr( new Reader(it->second) );
}

/*
  **********************************************
  L o c a l  D a t a  S t o r e  ::  R e a d e r
  **********************************************
*/

LocalDataStore::Reader::Reader(const std::shared_ptr<DataEntry>& data)
	: _data(data)
	, _offset(0)
{
}

bool LocalDataStore::Reader::seek(unsigned long long offset)
{
	if (offset > _data->data.size())
		return false;
	_offset = offset;
	return true;
}

int LocalDataStore::Reader::read(IByteStream& out)
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

const DataEntry& LocalDataStore::Reader::data() const
{
	return *_data;
}
/*
  </end child class>
*/

bool LocalDataStore::erase(const string& filename)
{
	// TODO: reader/writer locks.
	// this is the only "writer"
	return _store.unsafe_erase(filename) > 0;
}

std::string LocalDataStore::toString() const
{
	std::vector<string> report;
	for (data_map_type::const_iterator it = _store.begin(); it != _store.end(); ++it)
		report.push_back("(" + it->first + ")=>" + StringUtil::str(it->second->data.size()));
	std::sort(report.begin(), report.end());
	return StringUtil::stlJoin(report, '\n');
}
