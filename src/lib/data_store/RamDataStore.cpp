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
	// atomically increment the next version here???
	// needs to happen somewhere...
	return IDataStoreWriter::ptr(new Writer(filename, *this));
}

std::shared_ptr<IDataStoreWriter> RamDataStore::write(const string& filename, const string& version)
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

KeyMetadata& RamDataStore::Writer::metadata()
{
	return _data.md;
}
/*
  </end child class>
*/

IDataStoreReader::ptr RamDataStore::commit(Writer& writer)
{
	shared_ptr<DataEntry>& data = _store[writer.move_filename()];
	data.reset(new DataEntry(writer.move_data()));
	return IDataStoreReader::ptr(new Reader(data));
}

shared_ptr<IDataStoreReader> RamDataStore::read(const string& filename) const
{
	data_map_type::const_iterator it = _store.find(filename);
	if (it == _store.end())
		return NULL;

	return IDataStoreReader::ptr( new Reader(it->second) );
}

/*
  **********************************************
  R a m  D a t a  S t o r e  ::  R e a d e r
  **********************************************
*/

RamDataStore::Reader::Reader(const std::shared_ptr<DataEntry>& data)
	: _data(data)
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
/*
  </end child class>
*/

bool RamDataStore::erase(const string& filename)
{
	// TODO: reader/writer locks.
	// this is the only "writer"
	return _store.unsafe_erase(filename) > 0;
}

void RamDataStore::report(IByteStream& writer, const string& exclude) const
{
	bool first = true;
	for (data_map_type::const_iterator it = _store.begin(); it != _store.end(); ++it)
	{
		if (!exclude.empty() && it->first.find(exclude) != string::npos)
			continue;

		string fileReport = "(" + it->first + ")=>" + StringUtil::str(it->second->data.size());
		if (!first)
			fileReport = "\n" + fileReport;
		first &= false;
		writer.write(fileReport.data(), fileReport.size());
	}
}
