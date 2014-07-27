/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class IHashRing;
class IMembership;
class Peer;
class WriteParams;

class RandomizedMirrorToPeer
{
public:
	RandomizedMirrorToPeer(const IHashRing& ring, const IMembership& membership);

	bool chooseMirror(WriteParams& params, std::shared_ptr<Peer>& peer);

protected:
	const IMembership& _membership;
};
