/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
 
#include "data_structures/bounded_version_vector.h"

using VectorClock = bounded_version_vector<std::string,10>;
