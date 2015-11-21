/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
class IConsistentHashRing;
class IKnowPeers;
class IKeyTabulator;
class WriteInstructions;
class readstream;

class ModifyPeer
{
public:
	ModifyPeer(IConsistentHashRing& ring, IKnowPeers& membership, IKeyTabulator& keyTabulator);

	bool run(WriteInstructions& params, readstream& contents);

protected:
	bool update(const std::string& uid, readstream& contents);
	bool remove(const std::string& uid);

protected:
	IConsistentHashRing& _ring;
	IKnowPeers& _membership;
	IKeyTabulator& _keyTabulator;
};
