/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "deskew/ICorrectSkew.h"
#include "util/CallHistory.h"

class MockSkewCorrector : public ICorrectSkew
{
public:
	void healKey(const Peer& peer, const TreeId& treeid, unsigned long long key);
	void pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom);
	bool sendKey(const Peer& peer, const std::string& name, const std::string& version, const std::string& source);

public:
	CallHistory _history;
};
