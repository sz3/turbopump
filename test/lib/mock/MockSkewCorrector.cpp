/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockSkewCorrector.h"

#include "cohesion/TreeId.h"
#include "membership/Peer.h"

void MockSkewCorrector::healKey(const Peer& peer, unsigned long long key)
{
	_history.call("healKey", peer.uid, key);
}

void MockSkewCorrector::pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom)
{
	_history.call("pushKeyRange", peer.uid, treeid.id, first, last, offloadFrom);
}
