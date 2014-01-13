/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockDataStore.h"

#include "serialize/StringUtil.h"
#include "socket/IByteStream.h"
using std::string;

std::shared_ptr<IDataStoreWriter> MockDataStore::write(const string& filename)
{
	return IDataStoreWriter::ptr(new Writer(filename, *this));
}

/*
  ******************************************
  M o c k  D a t a  S t o r e  ::  W r i t e r
  ******************************************
*/
MockDataStore::Writer::Writer(std::string filename, MockDataStore& store)
	: _filename(std::move(filename))
	, _store(store)
{
}

bool MockDataStore::Writer::write(const char* buffer, unsigned size)
{
	_store._history.call("Writer::write", _filename, string(buffer, size));
	_buffer.append(buffer, size);
	return true;
}

IDataStoreReader::ptr MockDataStore::Writer::commit()
{
	_store._history.call("Writer::commit", _filename);
	return _store.commit(*this);
}

/*
  </end child class>
*/

IDataStoreReader::ptr MockDataStore::commit(Writer& writer)
{
	_store[writer._filename] = writer._buffer;
	return IDataStoreReader::ptr(new Reader(writer._buffer));
}

std::shared_ptr<IDataStoreReader> MockDataStore::read(const string& filename) const
{
	data_map_type::const_iterator it = _store.find(filename);
	if (it == _store.end())
		return NULL;

	return IDataStoreReader::ptr( new Reader(it->second) );
}

/*
  ******************************************
  M o c k  D a t a  S t o r e  ::  R e a d e r
  ******************************************
*/

MockDataStore::Reader::Reader(const std::string& data)
	: _data(data)
	, _offset(0)
{
}

bool MockDataStore::Reader::seek(unsigned long long offset)
{
	if (offset > _data.size())
		return false;
	_offset = offset;
	return true;
}

int MockDataStore::Reader::read(IByteStream& out)
{
	long long numBytes = _data.size() - _offset;
	if (numBytes <= 0)
		return 0;

	if (out.maxPacketLength() < numBytes)
		numBytes = out.maxPacketLength();

	const char* start = &(_data[_offset]);
	_offset += numBytes;
	return out.write(start, numBytes);
}
/*
  </end child class>
*/

bool MockDataStore::erase(const string& filename)
{
	_history.call("erase", filename);
	return _store.erase(filename) > 0;
}

std::string MockDataStore::toString() const
{
	std::vector<string> report;
	for (data_map_type::const_iterator it = _store.begin(); it != _store.end(); ++it)
		report.push_back("(" + it->first + ")=>" + it->second);
	return StringUtil::stlJoin(report, '\n');
}
