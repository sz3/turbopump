/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
class IDigestKeys;

class IDigestIndexer
{
public:
	virtual ~IDigestIndexer() {}

	virtual void update(const std::string& key, unsigned long long value) = 0;
	virtual void remove(const std::string& key) = 0;

	virtual void splitSection(const std::string& where) = 0;
	virtual void cannibalizeSection(const std::string& where) = 0;

	virtual const IDigestKeys& find(const std::string& id) const = 0;
	virtual const IDigestKeys& randomTree() const = 0;
	virtual const IDigestKeys& unwantedTree() const = 0;
};
