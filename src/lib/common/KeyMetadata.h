#pragma once

#include "VectorClock.h"
#include "common/turbopump_defaults.h"

struct KeyMetadata
{
	VectorClock version; // report to user as hash?
	unsigned short totalCopies = DEFAULT_MIRRORS;
	bool supercede = true;
	unsigned long long digest = 0;
};
