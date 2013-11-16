#include "MockSkewCorrector.h"

#include "membership/Peer.h"

void MockSkewCorrector::healKey(const Peer& peer, unsigned long long key)
{
	_history.call("healKey", peer.toString(), key);
}

void MockSkewCorrector::requestKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
{
	_history.call("requestKeyRange", peer.toString(), first, last);
}
