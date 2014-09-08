/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
class ILocateKeys;
class IMembership;
class Peer;
class WriteParams;

class MockMirrorToPeer
{
public:
	MockMirrorToPeer(const ILocateKeys& locator, const IMembership& membership);

	bool chooseMirror(WriteParams& params, std::shared_ptr<Peer>& peer);

	static void failOnce();
	static std::string calls();

protected:
};
