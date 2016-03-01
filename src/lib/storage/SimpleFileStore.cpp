/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SimpleFileStore.h"

#include "FileReader.h"
#include "FileWriter.h"

#include "readstream.h"
#include "writestream.h"
#include "common/turbopump_defaults.h"
#include "common/KeyMetadata.h"
#include "common/MyMemberId.h"
#include "common/WallClock.h"
#include "hashing/Hash.h"

#include "file/File.h"
#include "serialize/str.h"
#include "serialize/str_join.h"
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_set>

#include <boost/filesystem.hpp>
#include "dirent.h"
using std::shared_ptr;
using std::string;
using namespace std::placeholders;

namespace {
	VectorClock lookupVersion(const std::string& path, bool isDeleted=false)
	{
		uint64_t mtime = File::modified_time(path);
		VectorClock vc;
		vc.increment("sec", mtime);

		if (isDeleted)
			vc.markDeleted(mtime);
		return vc;
	}

	unsigned long long removeIfExists(const std::string& path, bool isDeleted=false)
	{
		unsigned long long digest = 0;
		if ( File::exists(path) )
		{
			VectorClock obsolete = lookupVersion(path, isDeleted);
			digest = writestream::digest(obsolete.toString(), File::size(path));
			File::remove(path);
		}
		return digest;
	}

	uint64_t timeFromVersion(const VectorClock& version)
	{
		if (version.empty())
			return 0;
		return version.clocks().front().time;
	}

	bool testIfVersionGreater(const VectorClock& version, const std::string& path)
	{
		if ( !File::exists(path) )
			return true;

		uint64_t prev = File::modified_time(path);
		uint64_t current = timeFromVersion(version);
		return current > prev || (current == prev && version.isDeleted());
	}
}

SimpleFileStore::SimpleFileStore(const std::string& homedir)
	: _homedir(homedir)
{
	// if homedir does not exist, make it
	boost::filesystem::create_directories(_homedir);
}

FilePaths SimpleFileStore::filepath(const std::string& name) const
{
	return FilePaths(_homedir, name);
}

VectorClock SimpleFileStore::mergedVersion(const std::string& name, bool inprogress/*=true*/) const
{
	VectorClock version;
	std::vector<std::string> vs(versions(name, inprogress));
	for (auto it = vs.begin(); it != vs.end(); ++it)
	{
		VectorClock temp;
		temp.fromString(*it);
		version.merge(temp);
	}
	return version;
}

writestream SimpleFileStore::write(const std::string& name, const std::string& version, unsigned short, unsigned long long offset)
{
	KeyMetadata md;
	md.version.fromString(version);
	if (md.version.empty())
		md.version.increment("sec");

	uint64_t current = timeFromVersion( mergedVersion(name, true) );
	uint64_t prospective = timeFromVersion(md.version);
	if ( prospective < current || (prospective == current && !md.version.isDeleted()) || md.version.isExpired(EXPIRY_TIMEOUT_SECONDS) )
		return writestream();
	// correct versions in the future back to now?

	FilePaths paths = filepath(name);
	string tempname(paths.inprogress());
	boost::filesystem::create_directories(boost::filesystem::path(tempname).parent_path());

	FileWriter* writer = new FileWriter(tempname);
	return writestream(writer, md, std::bind(&SimpleFileStore::onWriteComplete, this, name, _1));
}

readstream SimpleFileStore::read(const std::string& name, const std::string& version, bool inprogress/*=false*/) const
{
	// can't read inprogress versions, because I don't want to think about it.
	FilePaths paths = filepath(name);

	KeyMetadata md;
	if (!version.empty())
	{
		md.version.fromString(version);
		if (md.version.compare(mergedVersion(name)) != VectorClock::EQUAL)
			return readstream();
	}
	else
		md.version = mergedVersion(name);

	string filename = md.version.isDeleted()? paths.deleted() : paths.current();
	FileReader* reader = new FileReader(filename);
	if (name.find(MEMBERSHIP_FILE_PREFIX) == 0)
		md.totalCopies = 0;
	return readstream(reader, md);
}

std::vector<readstream> SimpleFileStore::readAll(const std::string& name) const
{
	std::vector<readstream> all;
	readstream current = read(name);
	if (current)
		all.push_back(current);
	return all;
}

bool SimpleFileStore::exists(const std::string& name, const std::string& version) const
{
	VectorClock v;
	v.fromString(version);

	VectorClock::COMPARE c = v.compare(mergedVersion(name));
	return c == VectorClock::EQUAL || c == VectorClock::LESS_THAN;
}

std::vector<std::string> SimpleFileStore::versions(const std::string& name, bool inprogress/*=false*/) const
{
	std::vector<std::string> versions;

	FilePaths paths = filepath(name);
	if ( File::exists(paths.current()) )
		versions.push_back( lookupVersion(paths.current()).toString() );

	if ( File::exists(paths.deleted()) )
		versions.push_back( lookupVersion(paths.deleted(), true).toString() );

	if ( inprogress && File::exists(paths.inprogress()) )
		versions.push_back( lookupVersion(paths.inprogress()).toString() );

	return versions;
}

bool SimpleFileStore::isExpired(const std::string& version) const
{
	VectorClock vc;
	if ( !vc.fromString(version) )
		return false;

	return vc.isExpired(EXPIRY_TIMEOUT_SECONDS);
}

bool SimpleFileStore::remove(const std::string& name)
{
	FilePaths fps = filepath(name);
	bool success = File::remove(fps.inprogress()) | File::remove(fps.deleted()) | File::remove(fps.current());

	boost::filesystem::path path = fps.current();
	path = path.parent_path();
	while (File::remove(path.string()))
		path = path.parent_path();

	return success;
}

bool SimpleFileStore::onWriteComplete(const std::string& name, KeyMetadata& md)
{
	if (md.version.isDeleted())
		return onDeleteComplete(name, md);

	FilePaths paths = filepath(name);
	if ( !testIfVersionGreater(md.version, paths.deleted()) )
	{
		File::remove(paths.inprogress());
		return false;
	}

	File::rename(paths.current(), paths.reserve());
	if ( !File::set_modified_time(paths.inprogress(), timeFromVersion(md.version)) || !File::rename(paths.inprogress(), paths.current()) )
	{
		File::rename(paths.reserve(), paths.current());
		File::remove(paths.inprogress());
		return false;
	}

	md.digest ^= removeIfExists(paths.reserve());
	md.digest ^= removeIfExists(paths.deleted(), true);
	return true;
}

bool SimpleFileStore::onDeleteComplete(const std::string& name, KeyMetadata& md)
{
	FilePaths paths = filepath(name);
	if ( !testIfVersionGreater(md.version, paths.current()) )
	{
		File::remove(paths.inprogress());
		return false;
	}

	unsigned long long currentDigest = removeIfExists(paths.deleted(), true);
	if ( !File::set_modified_time(paths.inprogress(), timeFromVersion(md.version)) || !File::rename(paths.inprogress(), paths.deleted()) )
	{
		File::remove(paths.inprogress());
		return false;
	}

	md.digest ^= removeIfExists(paths.current());
	md.digest ^= currentDigest;
	return true;
}

void SimpleFileStore::enumerate(const std::function<bool(const std::string&, const KeyMetadata&, const std::string&)> callback, unsigned long long limit) const
{
	// ignore in-progress versions
	// treat deleted versions as independent. We can clean up (probably) if/when we hit the real one.
	unsigned long long i = 0;
	for (boost::filesystem::recursive_directory_iterator it(_homedir, boost::filesystem::symlink_option::recurse), end; it != end; ++it)
	{
		boost::filesystem::path pa = it->path();
		if ( boost::filesystem::is_directory(pa) )
			continue;

		string filename = pa.string();
		if (filename.back() == '~')
			continue; // inprogress file

		string shortname;
		if (filename.find(_homedir) != string::npos)
			shortname = filename.substr(_homedir.size()+1);
		else
			shortname = pa.filename().string();

		bool isDeleted = shortname.find(".deleted.") == 0;
		if (isDeleted)
			shortname = shortname.substr(9);

		KeyMetadata md;
		md.version = lookupVersion(filename, isDeleted);
		string versionString = md.version.toString();
		unsigned long long size = File::size(filename);
		md.digest ^= writestream::digest(versionString, size);

		string report = turbo::str::str(size) + ":" + versionString;
		if (shortname.find(MEMBERSHIP_FILE_PREFIX) == 0)
			md.totalCopies = 0;

		callback(shortname, md, report);
		if (++i >= limit)
			break;
	}
}
