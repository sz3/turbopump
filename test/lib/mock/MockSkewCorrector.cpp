#include "MockSkewCorrector.h"

#include "membership/Peer.h"

void MockSkewCorrector::healKey(const Peer& peer, unsigned long long key)
{
	_history.call("healKey", peer.uid, key);
}

void MockSkewCorrector::pushKeyRange(const Peer& peer, unsigned long long first, unsigned long long last)
{
	_history.call("pushKeyRange", peer.uid, first, last);
}
