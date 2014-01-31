/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"

class IHashRing;
class IMembership;
class IKeyTabulator;

class RemovePeerAction : public IAction
{
public:
	RemovePeerAction(IHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	IHashRing& _ring;
	IMembership& _membership;
	IKeyTabulator& _keyTabulator;

	std::string _uid;
};
