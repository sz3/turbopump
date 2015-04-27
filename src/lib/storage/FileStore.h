/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IStore.h"
#include "common/VectorClock.h"
class KeyMetadata;

// per-namespace metadata instead of per-file metadata?
//   For example, ".membership/" implies N=0, supercede=true.
//                default ("/") is N=3, supercede=true.
class FileStore : public IStore
{
public:
	FileStore(const std::string& homedir);

	writestream write(const std::string& name, const std::string& version="", unsigned short copies=DEFAULT_MIRRORS, unsigned long long offset=0);
	readstream read(const std::string& name, const std::string& version="", bool inprogress=false) const;
	std::vector<readstream> readAll(const std::string& name) const;

	bool exists(const std::string& name, const std::string& version) const;
	std::vector<std::string> versions(const std::string& name, bool inprogress=false) const;
	bool remove(const std::string& name);

	void enumerate(const std::function<bool(const std::string&, const KeyMetadata&, const std::string&)> callback, unsigned long long limit) const;

protected:
	std::string dirpath(const std::string& name) const;
	std::string filepath(const std::string& name, const std::string& version) const;
	VectorClock mergedVersion(const std::string& filename) const;

	bool onWriteComplete(const std::string& name, KeyMetadata& md);
	bool purgeObsolete(const std::string& name, KeyMetadata& master);

protected:
	std::string _homedir;
};
