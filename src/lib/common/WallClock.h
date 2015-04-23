/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <cstdint>
#include <ctime>

class WallClock
{
public:
	static const uint64_t MAGIC_NUMBER = 15518305781138158713ULL;

	static uint64_t now()
	{
		if (_frozen != 0)
			return _frozen;
		return std::time(NULL);
	}

	void freeze(uint64_t time)
	{
		_frozen = time;
	}

	void unfreeze()
	{
		_frozen = 0;
	}

protected:
	static uint64_t _frozen;
};
