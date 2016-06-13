/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
class Peer;
class TreeId;

class ICorrectSkew
{
public:
	virtual ~ICorrectSkew() {}

	virtual bool dropKey(const std::string& name) = 0;

	virtual void pushKey(const Peer& peer, const TreeId& treeid, unsigned long long key) = 0;
	virtual void pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom="") = 0;
	virtual bool sendKey(const Peer& peer, const std::string& name, const std::string& version, unsigned long long offset, const std::string& source) = 0;
};
