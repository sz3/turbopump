/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"

class IHashRing;
class IMembership;
class IKeyTabulator;
#include <memory>

class AddPeerAction : public IAction
{
public:
	AddPeerAction(std::unique_ptr<IAction>&& writeAction);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	std::unique_ptr<IAction> _writeAction;

	std::string _uid;
	std::string _ip;
};
