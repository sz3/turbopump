#pragma once

#include "MockStoreReader.h"
#include "MockStoreWriter.h"
#include "storage/IStore.h"
#include "util/CallHistory.h"
#include <map>

class MockStore : public IStore
{
public:
	writestream write(const std::string& name, const std::string& version="", unsigned short copies=3, unsigned long long offset=0);
	readstream read(const std::string& name, const std::string& version, bool inprogress) const;
	std::vector<readstream> readAll(const std::string& name) const;

	bool exists(const std::string& name, const std::string& version="") const;
	std::vector<std::string> versions(const std::string& name, bool inprogress=false) const;
	bool remove(const std::string& name);

	void enumerate(const std::function<bool(const std::string&, const KeyMetadata&, const std::string&)> callback, unsigned long long limit) const;

public:
	mutable CallHistory _history;

	std::map<std::string, std::string> _reads;
	MockStoreWriter* _writer = NULL;
	bool _exists = true;
	std::vector<std::string> _versions;
};

