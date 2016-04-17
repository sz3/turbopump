/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "KeyRange.h"

#include <endian.h>
#include <iostream>

KeyRange::KeyRange(const MerklePoint& location)
{
	fromLocation(location);
}

void KeyRange::fromLocation(const MerklePoint& location)
{
	if (location.keybits >= 64)
	{
		_first = _last = location.key;
		return;
	}
	else if (location.keybits == 0)
	{
		_first = 0;
		_last = ~0ULL;
		return;
	}

	unsigned shift = (sizeof(location.key)<<3) - location.keybits;
	_first = htobe64(location.key);
	_first = (_first >> shift) << shift;
	_last = _first xor (~0ULL >> location.keybits);

	_first = be64toh(_first);
	_last = be64toh(_last);
}

unsigned long long KeyRange::first() const
{
	return _first;
}

unsigned long long KeyRange::last() const
{
	return _last;
}
