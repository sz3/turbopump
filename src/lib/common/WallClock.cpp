/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WallClock.h"

uint64_t WallClock::_frozen = 0;
const uint64_t WallClock::MAGIC_NUMBER;
