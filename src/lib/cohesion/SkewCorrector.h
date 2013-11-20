#pragma once

#include "ICorrectSkew.h"

class SkewCorrector : public ICorrectSkew
{
public:
	void healKey(const Peer& peer, unsigned long long key);
	void pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last);

protected:
};
