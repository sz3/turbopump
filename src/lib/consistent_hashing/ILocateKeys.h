/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
#include <vector>

class ILocateKeys
{
public:
	virtual ~ILocateKeys() {}

	virtual std::vector<std::string> locations(const std::string& name, unsigned mirrors) const = 0;
	virtual bool containsSelf(const std::vector<std::string>& locs) const = 0;
	virtual bool keyIsMine(const std::string& name, unsigned mirrors) const = 0;
};
