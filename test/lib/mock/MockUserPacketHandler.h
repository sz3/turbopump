/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "user_server/IUserPacketHandler.h"
#include "util/CallHistory.h"

class MockUserPacketHandler : public IUserPacketHandler
{
public:
	MockUserPacketHandler();

	void sendResponse(StatusCode status);
	std::unique_ptr<IAction> newAction(const std::string& actionName, const std::map<std::string,std::string>& params) const;

public:
	mutable CallHistory _history;
	mutable IAction* _action;
};
