/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "consistent_hashing/ILocateKeys.h"
#include "util/CallHistory.h"

class MockLocateKeys : public ILocateKeys
{
public:
	MockLocateKeys();

	std::vector<std::string> locations(const std::string& name, unsigned mirrors) const;
	std::string section(const std::string& name) const;

	bool containsSelf(const std::vector<std::string>& locs) const;
	bool keyIsMine(const std::string& name, unsigned mirrors) const;
	bool sectionIsMine(const std::string& id, unsigned mirrors) const;

public:
	mutable CallHistory _history;
	std::vector<std::string> _locations;
	bool _mine;
};
