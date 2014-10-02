/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "RandomizedMirrorToPeer.h"

#include "api/WriteInstructions.h"
#include "membership/IMembership.h"

RandomizedMirrorToPeer::RandomizedMirrorToPeer(const ILocateKeys& locator, const IMembership& membership)
	: _membership(membership)
{
}

bool RandomizedMirrorToPeer::chooseMirror(WriteInstructions& params, std::shared_ptr<Peer>& peer)
{
	if (params.mirror >= params.copies)
		return false;

	peer = _membership.randomPeer();
	return !!peer;
}
