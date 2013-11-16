#pragma once

#include "TurboApi.h"

class IMembership;
class IPeerTracker;

// not really sure about the name of this guy, yet
class Callbacks : public TurboApi
{
public:
	Callbacks(const TurboApi& instruct);

	void initialize(const IMembership& membership, IPeerTracker& peers);

protected:
};

