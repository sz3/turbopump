/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"

class IHashRing;
class IMembership;
class IMerkleIndex;

class RemovePeerAction : public IAction
{
public:
	RemovePeerAction(IHashRing& ring, IMembership& membership, IMerkleIndex& merkleIndex);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	IHashRing& _ring;
	IMembership& _membership;
	IMerkleIndex& _merkleIndex;

	std::string _uid;
};
