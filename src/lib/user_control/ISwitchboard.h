/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

class ISwitchboard
{
public:
	~ISwitchboard() {}

	virtual void parse(const char* buffer, unsigned bytes) = 0;
};
