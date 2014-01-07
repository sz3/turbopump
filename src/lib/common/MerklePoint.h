/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "data_structures/merkle_location.h"
#include <string>

using MerklePoint = merkle_point<unsigned long long, unsigned long long>;

namespace MerklePointSerializer
{
	std::string toString(const MerklePoint& point);
	bool fromString(MerklePoint& point, const std::string& str);
}
