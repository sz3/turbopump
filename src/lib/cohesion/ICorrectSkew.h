#pragma once

class Peer;

class ICorrectSkew
{
public:
	virtual ~ICorrectSkew() {}

	virtual void healKey(const Peer& peer, unsigned long long key) = 0;
	virtual void requestKeyRange(const Peer& peer, unsigned long long first, unsigned long long last) = 0;
};
