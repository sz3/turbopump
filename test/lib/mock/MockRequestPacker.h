/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "actions_req/IMessagePacker.h"
#include "util/CallHistory.h"

class MockRequestPacker : public IMessagePacker
{
public:
	std::string package(unsigned char id, const char* packet, unsigned size) const;

public:
	mutable CallHistory _history;
};

