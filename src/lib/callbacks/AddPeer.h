/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

class IConsistentHashRing;
class IMembership;
class IKeyTabulator;
class WriteInstructions;
class readstream;

class AddPeer
{
public:
	AddPeer(IConsistentHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator);

	bool run(WriteInstructions& params, readstream& contents);

protected:
	IConsistentHashRing& _ring;
	IMembership& _membership;
	IKeyTabulator& _keyTabulator;
};
