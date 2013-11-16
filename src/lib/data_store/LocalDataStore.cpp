#include "LocalDataStore.h"

#include "IDataStoreWriter.h"
#include "socket/IByteStream.h"

#include <cstring>
#include <iostream>
#include <sstream>
using namespace std::placeholders;
using std::bind;
using std::shared_ptr;
using std::string;
using std::unordered_map;

std::shared_ptr<IDataStoreWriter> LocalDataStore::write(const string& filename)
{
	return IDataStoreWriter::ptr(new Writer(filename, *this));
}

/*
  ******************************************
  L o c a l  D a t a  S t o r e  ::  W r i t e r
  ******************************************
*/
LocalDataStore::Writer::Writer(const std::string& filename, LocalDataStore& store)
	: _filename(std::move(filename))
	, _store(store)
{
}

bool LocalDataStore::Writer::write(const char* buffer, unsigned size)
{
	_buffer.append(buffer, size);
	return true;
}

IDataStoreReader::ptr LocalDataStore::Writer::commit()
{
	return _store.commit(*this);
}

std::string&& LocalDataStore::Writer::filename()
{
	return std::move(_filename);
}

std::string&& LocalDataStore::Writer::buffer()
{
	return std::move(_buffer);
}
/*
  </end child class>
*/

IDataStoreReader::ptr LocalDataStore::commit(Writer& writer)
{
	shared_ptr<string>& data = _store[writer.filename()];
	data.reset(new string(writer.buffer()));
	return IDataStoreReader::ptr(new Reader(data));
}

shared_ptr<IDataStoreReader> LocalDataStore::read(const string& filename)
{
	unordered_map< string, shared_ptr<string> >::const_iterator it = _store.find(filename);
	if (it == _store.end())
		return NULL;

	return IDataStoreReader::ptr( new Reader(it->second) );
}

/*
  ******************************************
  L o c a l  D a t a  S t o r e  ::  R e a d e r
  ******************************************
*/

LocalDataStore::Reader::Reader(const std::shared_ptr<std::string>& data)
	: _data(data)
	, _offset(0)
{
}

bool LocalDataStore::Reader::seek(unsigned long long offset)
{
	if (offset > _data->size())
		return false;
	_offset = offset;
	return true;
}

int LocalDataStore::Reader::read(IByteStream& out)
{
	long long numBytes = _data->size() - _offset;
	if (numBytes <= 0)
		return 0;

	if (out.maxPacketLength() < numBytes)
		numBytes = out.maxPacketLength();

	const char* start = &(*_data)[_offset];
	_offset += numBytes;
	return out.write(start, numBytes);
}
/*
  </end child class>
*/

void LocalDataStore::erase(const string& filename, const std::function<void(bool)>& callback)
{
	size_t num = _store.erase(filename);
	callback(num > 0);
}

std::string LocalDataStore::toString() const
{
	std::stringstream report;
	report << "*** " << _store.size() << " keys ***" << std::endl;
	for (unordered_map< string, shared_ptr<string> >::const_iterator it = _store.begin(); it != _store.end(); ++it)
		report << " (" + it->first + ")=>" << *it->second << std::endl;
	report << "*** done ***" << std::endl;
	return report.str();
}
