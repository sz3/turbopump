/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ILocateKeys.h"
class IHashRing;
class IMembership;

class LocateKeys : public ILocateKeys
{
public:
	LocateKeys(const IHashRing& ring, const IMembership& membership);

	std::vector<std::string> locations(const std::string& name, unsigned mirrors) const;
	bool containsSelf(const std::vector<std::string>& locs) const;
	bool keyIsMine(const std::string& name, unsigned mirrors) const;

protected:
	const IHashRing& _ring;
	const IMembership& _membership;
};
