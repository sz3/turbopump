/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "merkle_tree/merkle_location.h"

class KeyRange
{
public:
	KeyRange();
	KeyRange(const turbo::merkle_location<unsigned long long>& location);

	void fromLocation(const turbo::merkle_location<unsigned long long>& location);

	unsigned long long first() const;
	unsigned long long last() const;

protected:
	unsigned long long _first;
	unsigned long long _last;
};

