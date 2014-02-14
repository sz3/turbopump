#pragma once

#include "VectorClock.h"

struct KeyMetadata
{
	VectorClock version; // report to user as hash?
	unsigned short totalCopies;
};
