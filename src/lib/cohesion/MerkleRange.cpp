#include "MerkleRange.h"

#include <endian.h>

MerkleRange::MerkleRange(const merkle_location<unsigned long long>& location)
{
	fromLocation(location);
}

void MerkleRange::fromLocation(const merkle_location<unsigned long long>& location)
{
	if (location.keybits >= 63)
	{
		_first = _last = location.key;
		return;
	}

	unsigned shift = (sizeof(location.key)<<3) - location.keybits-1;
	_first = htobe64(location.key);
	_first = (_first >> shift) << shift;
	_last = _first xor (~0ULL >> (location.keybits+1));

	_first = be64toh(_first);
	_last = be64toh(_last);
}

unsigned long long MerkleRange::first() const
{
	return _first;
}

unsigned long long MerkleRange::last() const
{
	return _last;
}
