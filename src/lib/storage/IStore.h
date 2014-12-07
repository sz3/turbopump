/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

// I really don't like returning a std::shared_ptr<Interface>, but I don't know of a better way to do runtime polymorphism...
// fix this, Bjarne!

#include "IReader.h"
#include "IWriter.h"
#include "common/KeyMetadata.h"

#include <functional>
#include <string>
#include <vector>
class readstream;
class writestream;

class IStore
{
public:
	virtual ~IStore() {}

	virtual writestream write(const std::string& name, const std::string& version="", unsigned long long offset=0) = 0;
	virtual readstream read(const std::string& name, const std::string& version="") const = 0;
	virtual std::vector<readstream> readAll(const std::string& name) const = 0;
	
	virtual bool exists(const std::string& name, const std::string& version="") const = 0;
	virtual std::vector<std::string> versions(const std::string& name, bool inprogress=false) const = 0;
	virtual bool remove(const std::string& name) = 0;

	virtual void enumerate(const std::function<bool(const std::string&)> callback, unsigned limit) const = 0;
};
