/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockStore.h"

#include "common/KeyMetadata.h"
#include "storage/readstream.h"
#include "storage/writestream.h"
#include "storage/StringReader.h"
using std::map;
using std::string;

writestream MockStore::write(const std::string& name, const std::string& version, unsigned short copies, unsigned long long offset)
{
	_history.call("write", name, version, offset);
	KeyMetadata md;
	md.version.fromString(version);
	if (md.version.empty())
		md.version.increment("mock");

	IWriter* writer = _writer;
	_writer = NULL;
	return writestream(writer, md);
}

readstream MockStore::read(const std::string& name, const std::string& version, bool inprogress) const
{
	_history.call("read", name, version);
	KeyMetadata md;

	map<string, string>::const_iterator it = _reads.find(name);
	if (it == _reads.end())
		return readstream(NULL, md);

	md.version.fromString(version);
	return readstream(new StringReader(it->second), md);
}

std::vector<readstream> MockStore::readAll(const std::string& name) const
{
	_history.call("readAll", name);
	std::vector<readstream> all;
	for (auto version : _versions)
	{
		KeyMetadata md;
		md.version.fromString(version);

		map<string, string>::const_iterator it = _reads.find(name);
		if (it != _reads.end())
			all.push_back( readstream(new StringReader(it->second), md) );
	}
	return all;
}

bool MockStore::exists(const std::string& name, const std::string& version) const
{
	_history.call("exists", name, version);
	return _exists;
}

std::vector<std::string> MockStore::versions(const std::string& name, bool inprogress) const
{
	_history.call("versions", name, inprogress);
	return _versions;
}

bool MockStore::isExpired(const std::string& version) const
{
	_history.call("isExpired", version);
	return false;
}

bool MockStore::remove(const std::string& name)
{
	_history.call("remove", name);
	return true;
}

void MockStore::enumerate(const std::function<bool(const std::string&, const KeyMetadata&, const std::string&)> callback, unsigned long long limit) const
{
	_history.call("enumerate", limit);
}
