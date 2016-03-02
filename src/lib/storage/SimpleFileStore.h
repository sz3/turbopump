/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IStore.h"
#include "common/VectorClock.h"
#include "file/File.h"
#include <boost/filesystem.hpp>
class KeyMetadata;

class FilePaths
{
public:
	FilePaths(const std::string& homedir, const std::string& name)
	{
		boost::filesystem::path basepath(homedir);
		boost::filesystem::path filename(name);
		basepath /= filename.parent_path();
		_name = filename.filename().string();
		_homedir = basepath.string() + "/";
	}

	std::string current() const
	{
		return _homedir + _name;
	}

	std::string inprogress() const
	{
		return _homedir + "." + _name + "~";
	}

	std::string deleted() const
	{
		return _homedir + ".deleted." + _name;
	}

	std::string reserve() const
	{
		return inprogress() + "~";
	}

protected:
	std::string _homedir;
	std::string _name;
};

// per-namespace metadata instead of per-file metadata?
//   For example, ".membership/" implies N=0, supercede=true.
//                default ("/") is N=3, supercede=true.
class SimpleFileStore : public IStore
{
public:
	SimpleFileStore(const std::string& homedir);

	writestream write(const std::string& name, const std::string& version="", unsigned short copies=DEFAULT_MIRRORS, unsigned long long offset=0);
	readstream read(const std::string& name, const std::string& version="", bool inprogress=false) const;
	std::vector<readstream> readAll(const std::string& name) const;

	bool exists(const std::string& name, const std::string& version) const;
	std::vector<std::string> versions(const std::string& name, bool inprogress=false) const;
	bool isExpired(const std::string& version) const;
	bool remove(const std::string& name);

	void enumerate(const std::function<bool(const std::string&, const KeyMetadata&, const std::string&)> callback,
				   unsigned long long limit, const std::string& prefix="") const;

protected:
	FilePaths filepath(const std::string& name) const;
	VectorClock mergedVersion(const std::string& filename, bool inprogress=true) const;

	bool onWriteComplete(const std::string& name, KeyMetadata& md);
	bool onDeleteComplete(const std::string& name, KeyMetadata& md);

protected:
	std::string _homedir;
};
