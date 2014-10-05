/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

class IMessagePacker
{
public:
	virtual ~IMessagePacker() {}

	virtual std::string package(unsigned char id, const char* packet, unsigned size) const = 0;
};
