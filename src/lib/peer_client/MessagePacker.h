/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IMessagePacker.h"

class MessagePacker : public IMessagePacker
{
public:
	std::string package(unsigned char id, const char* packet, unsigned size) const;
	std::string package(unsigned char id, const std::string& packet) const;

protected:
	void pack(char* buffer, unsigned char id, const char* packet, unsigned size) const;
};
