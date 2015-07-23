/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockSkewCorrector.h"

#include "deskew/TreeId.h"
#include "membership/Peer.h"

bool MockSkewCorrector::dropKey(const std::string& name)
{
	_history.call("dropKey", name);
	return true;
}

void MockSkewCorrector::pushKey(const Peer& peer, const TreeId& treeid, unsigned long long key)
{
	_history.call("pushKey", peer.uid, treeid.id, key);
}

void MockSkewCorrector::pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom)
{
	_history.call("pushKeyRange", peer.uid, treeid.id, treeid.mirrors, first, last, offloadFrom);
}

bool MockSkewCorrector::sendKey(const Peer& peer, const std::string& name, const std::string& version, const std::string& source)
{
	_history.call("sendKey", peer.uid, name, version, source);
	return true;
}
