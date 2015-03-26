/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class ILocateKeys;
class IMembership;
class Peer;
class WriteInstructions;

class RandomizedMirrorToPeer
{
public:
	RandomizedMirrorToPeer(const ILocateKeys& locator, const IMembership& membership);

	bool chooseMirror(WriteInstructions& params, std::shared_ptr<Peer>& peer);

protected:
	const IMembership& _membership;
};