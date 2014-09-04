/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_structures/merkle_location.h"

class KeyRange
{
public:
	KeyRange();
	KeyRange(const merkle_location<unsigned long long>& location);

	void fromLocation(const merkle_location<unsigned long long>& location);

	unsigned long long first() const;
	unsigned long long last() const;

protected:
	unsigned long long _first;
	unsigned long long _last;
};

