/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ILocateKeys.h"
class IConsistentHashRing;
class IMembership;

class LocateKeys : public ILocateKeys
{
public:
	LocateKeys(const IConsistentHashRing& ring, const IMembership& membership);

	std::vector<std::string> locations(const std::string& name, unsigned limit) const;
	std::string section(const std::string& name) const;

	bool containsSelf(const std::vector<std::string>& locs) const;
	bool keyIsMine(const std::string& name, unsigned limit) const;
	bool sectionIsMine(const std::string& id, unsigned limit) const;

protected:
	const IConsistentHashRing& _ring;
	const IMembership& _membership;
};
