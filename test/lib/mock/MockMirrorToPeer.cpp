/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockMirrorToPeer.h"

#include "actions/WriteParams.h"
#include "membership/Peer.h"
#include "util/CallHistory.h"

namespace {
	bool _fail;
	CallHistory _history;
}

MockMirrorToPeer::MockMirrorToPeer(const IHashRing& ring, const IMembership& membership)
	: _ring(ring)
	, _membership(membership)
{
}

bool MockMirrorToPeer::chooseMirror(WriteParams& params, std::shared_ptr<Peer>& peer)
{
	_history.call("chooseMirror", params.filename);
	if (!_fail)
		peer.reset(new Peer("peerid"));
	_fail = false;
	return !!peer;
}

void MockMirrorToPeer::failOnce()
{
	_fail = true;
}

std::string MockMirrorToPeer::calls()
{
	std::string res = _history.calls();
	_history.clear();
	return res;
}
