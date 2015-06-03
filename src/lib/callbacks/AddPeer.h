/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

class IConsistentHashRing;
class IKnowPeers;
class IKeyTabulator;
class WriteInstructions;
class readstream;

class AddPeer
{
public:
	AddPeer(IConsistentHashRing& ring, IKnowPeers& membership, IKeyTabulator& keyTabulator);

	bool run(WriteInstructions& params, readstream& contents);

protected:
	IConsistentHashRing& _ring;
	IKnowPeers& _membership;
	IKeyTabulator& _keyTabulator;
};
