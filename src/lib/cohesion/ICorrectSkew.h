/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
class Peer;

class ICorrectSkew
{
public:
	virtual ~ICorrectSkew() {}

	virtual void healKey(const Peer& peer, unsigned long long key) = 0;
	virtual void pushKeyRange(const Peer& peer, const std::string& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom="") = 0;
};
