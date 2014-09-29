/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "http/StatusCode.h"
#include <memory>
#include <string>
#include <unordered_map>
namespace Turbopump { class Command; }

class IUserPacketHandler
{
public:
	~IUserPacketHandler() {}

	virtual void sendResponse(StatusCode status) = 0;
	virtual std::unique_ptr<Turbopump::Command> command(const std::string& cmd, const std::unordered_map<std::string,std::string>& params) const = 0;
};
