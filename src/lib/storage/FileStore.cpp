/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "FileStore.h"

#include "FileReader.h"
#include "FileWriter.h"

#include "readstream.h"
#include "writestream.h"
#include "common/turbopump_defaults.h"
#include "common/KeyMetadata.h"
#include "common/MyMemberId.h"
#include "hashing/Hash.h"

#include "file/File.h"
#include "serialize/str.h"
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include <boost/filesystem.hpp>
#include "dirent.h"
using std::shared_ptr;
using std::string;
using namespace std::placeholders;

namespace {
	// used to get all versions of a file == list all files in a file's directory.
	// names will likely be base85 encoded, but for now it's a simple VectorClock::toString().
	std::vector<std::string> list(const std::string& dir)
	{
		std::vector<std::string> entries;

		DIR* dirp = opendir(dir.c_str());
		if (dirp == NULL)
			return entries;

		struct dirent* dp;
		while ((dp = readdir(dirp)) != NULL)
			entries.push_back(dp->d_name);
		closedir(dirp);

		return entries;
	}

	bool mdFromString(KeyMetadata& md, const std::string& str)
	{
		if (str.empty())
			return false;
		md.totalCopies = (unsigned char)str[0];
		md.supercede = (str[1] == '1');
		return true;
	}

	std::string mdToString(const KeyMetadata& md)
	{
		std::string res;
		res += (unsigned char)md.totalCopies;
		if (md.supercede)
			res += "1";
		else
			res += "0";
		return res;
	}
}

FileStore::FileStore(const std::string& homedir)
	: _homedir(homedir)
{
	// if homedir does not exist, make it
	boost::filesystem::create_directories(_homedir);
}

std::string FileStore::dirpath(const std::string& name) const
{
	// eventually we'll base85 encode the Hash and use it for a filename. Probably.
	return _homedir + "/" + name;
}

std::string FileStore::filepath(const std::string& name, const std::string& version) const
{
	return dirpath(name) + "/" + version;
}

VectorClock FileStore::mergedVersion(const std::string& name) const
{
	VectorClock version;
	std::vector<std::string> vs(versions(name, true));
	for (auto it = vs.begin(); it != vs.end(); ++it)
	{
		VectorClock temp;
		temp.fromString(*it);
		version.merge(temp);
	}
	return version;
}

writestream FileStore::write(const std::string& name, const std::string& version, unsigned short copies, unsigned long long offset)
{
	// just version it at the outset! Why not?
	KeyMetadata md;
	md.version.fromString(version);
	if (md.version.empty())
	{
		md.version = mergedVersion(name);
		md.version.increment(MyMemberId());
	}
	else if ( exists(name, version) )
		return writestream();
	md.totalCopies = copies;

	string tempname(filepath(name, md.version.toString()) + "~");
	boost::filesystem::create_directories(dirpath(name));

	FileWriter* writer = new FileWriter(tempname);
	if (writer->good())
		writer->setAttribute("user.md", mdToString(md));
	return writestream(writer, md, std::bind(&FileStore::onWriteComplete, this, name, _1));
}

readstream FileStore::read(const std::string& name, const std::string& version) const
{
	KeyMetadata md;
	md.version.fromString(version);
	if (md.version.empty())
		md.version = mergedVersion(name);

	string filename(filepath(name, md.version.toString()));
	FileReader* reader = new FileReader(filename);
	if (reader->good())
		mdFromString(md, reader->attribute("user.md"));
	return readstream(reader, md);
}

std::vector<readstream> FileStore::readAll(const std::string& name) const
{
	std::vector<readstream> all;
	std::vector<std::string> vs(versions(name));
	for (const std::string& version : vs)
		all.push_back( read(name, version) );
	return all;
}

bool FileStore::exists(const std::string& name, const std::string& version) const
{
	std::vector<string> all(versions(name, true));
	return std::find(all.begin(), all.end(), version) != all.end();
}

std::vector<std::string> FileStore::versions(const std::string& name, bool inprogress/*=false*/) const
{
	// by default, in progress versions are ignored.

	std::vector<std::string> versions = list(dirpath(name));
	for (auto it = versions.begin(); it != versions.end();)
	{
		string& ver = *it;
		if (ver.find(',') == string::npos)
		{
			it = versions.erase(it);
			continue;
		}
		if (ver.back() == '~')
		{
			if (!inprogress)
			{
				it = versions.erase(it);
				continue;
			}
			ver = ver.substr(0, ver.size()-1);
		}
		++it;
	}
	return versions;
}

bool FileStore::remove(const std::string& name)
{
	boost::filesystem::remove_all(dirpath(name));
	return true;
}

bool FileStore::onWriteComplete(const std::string& name, KeyMetadata& md)
{
	string filename(filepath(name, md.version.toString()));
	string tempname = filename + "~";
	if ( !File::rename(tempname, filename) )
		return false;

	return purgeObsolete(name, md);
}

bool FileStore::purgeObsolete(const std::string& name, KeyMetadata& master)
{
	// TODO: this doesn't work with concurrent writes yet. Will lead to weird merkle sync problems.
	std::vector<std::string> all(versions(name));
	std::vector<std::string> toDelete;
	for (const std::string& v : all)
	{
		VectorClock version;
		version.fromString(v);
		if (version < master.version)
			toDelete.push_back(version.toString());
	}
	// TODO: optimize:
	// if all.size() == deleted.size() + 1
	//   shortcut digest computation (nothing to do)
	// else
	//   do what's below == compute writestream::digest() for each (name,vstr) and xor it into master.digest

	// then delete them
	for (const std::string& vstr : toDelete)
	{
		string path = filepath(name, vstr);
		master.digest ^= writestream::digest(vstr, File::size(path));
		File::remove(path);
	}
	return true;
}

void FileStore::enumerate(const std::function<bool(const std::string&, const KeyMetadata&, const std::string&)> callback, unsigned limit) const
{
	int i = 0;
	boost::filesystem::recursive_directory_iterator end;
	boost::filesystem::directory_iterator dend;
	for (boost::filesystem::recursive_directory_iterator it(_homedir); it != end; ++it)
	{
		boost::filesystem::path pa = it->path();
		if ( !boost::filesystem::is_directory(pa) )
			continue;

		KeyMetadata md;
		string report;
		for (boost::filesystem::directory_iterator version_it(pa); version_it != dend; ++version_it)
		{
			boost::filesystem::path vpath = version_it->path();
			if ( !boost::filesystem::is_directory(vpath) )
			{
				string vstr = vpath.filename().string();
				unsigned long long size = boost::filesystem::file_size(vpath);
				if (report.empty())
				{
					FileReader reader(vpath.string());
					if ( reader.good() )
						mdFromString(md, reader.attribute("user.md"));
				}
				md.digest ^= writestream::digest(vstr, size);
				report += " " + turbo::str::str(size) + "|" + vstr;
			}
		}
		if (report.empty())
			continue;

		string filename = pa.string();
		if (filename.find(_homedir) != string::npos)
			filename = filename.substr(_homedir.size()+1);
		else
			filename = pa.filename().string();

		// middle param == metadata. Version is nonsense, of course.
		callback(filename, md, report);
		if (++i >= limit)
			break;
	}
}
