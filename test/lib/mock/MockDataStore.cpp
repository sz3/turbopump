/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockDataStore.h"

#include "serialize/StringUtil.h"
#include "socket/IByteStream.h"
using std::string;

std::shared_ptr<IDataStoreWriter> MockDataStore::write(const string& filename)
{
	return IDataStoreWriter::ptr(new Writer(filename, *this));
}

std::shared_ptr<IDataStoreWriter> MockDataStore::write(const string& filename, const string& version)
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
	_data.md.totalCopies = 1;
}

bool MockDataStore::Writer::write(const char* buffer, unsigned size)
{
	_store._history.call("Writer::write", _filename, string(buffer, size));
	_data.data.append(buffer, size);
	return true;
}

IDataStoreReader::ptr MockDataStore::Writer::commit()
{
	_store._history.call("Writer::commit", _filename, _data.md.totalCopies);
	return _store.commit(*this);
}

KeyMetadata& MockDataStore::Writer::metadata()
{
	return _data.md;
}

/*
  </end child class>
*/

IDataStoreReader::ptr MockDataStore::commit(Writer& writer)
{
	_store[writer._filename] = writer._data.data;
	return IDataStoreReader::ptr(new Reader(writer._data.data));
}

std::vector<std::shared_ptr<IDataStoreReader> > MockDataStore::read(const string& filename) const
{
	_history.call("read", filename);
	std::vector<IDataStoreReader::ptr> reads;
	data_map_type::const_iterator it = _store.find(filename);
	if (it != _store.end())
		reads.push_back(IDataStoreReader::ptr(new Reader(it->second)));

	return reads;
}

std::shared_ptr<IDataStoreReader> MockDataStore::read(const string& filename, const string& version) const
{
	_history.call("read", filename, version);
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
	: _offset(0)
{
	_data.data = data;
	_data.md.totalCopies = 1;
}

bool MockDataStore::Reader::seek(unsigned long long offset)
{
	if (offset > _data.data.size())
		return false;
	_offset = offset;
	return true;
}

int MockDataStore::Reader::read(IByteStream& out)
{
	long long numBytes = _data.data.size() - _offset;
	if (numBytes <= 0)
		return 0;

	if (out.maxPacketLength() < numBytes)
		numBytes = out.maxPacketLength();

	const char* start = &(_data.data[_offset]);
	_offset += numBytes;
	return out.write(start, numBytes);
}

const KeyMetadata& MockDataStore::Reader::metadata() const
{
	return _data.md;
}
/*
  </end child class>
*/

bool MockDataStore::drop(const string& filename)
{
	_history.call("drop", filename);
	return _store.erase(filename) > 0;
}

void MockDataStore::report(IByteStream& writer, const string& exclude) const
{
	_history.call("report", exclude);
}
