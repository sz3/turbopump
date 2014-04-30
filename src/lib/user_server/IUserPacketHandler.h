/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <memory>
#include <string>
class IAction;

class IUserPacketHandler
{
public:
	~IUserPacketHandler() {}

	virtual std::unique_ptr<IAction> newAction(const std::string& actionName, const std::map<std::string,std::string>& params) const = 0;
};
