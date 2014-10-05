/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Command.h"
#include <queue>
#include <memory>
#include <string>

// TODO: name this better. It's more an action helper. Buffer. Thing.
class VirtualConnection
{
public:
	void setCommand(const std::shared_ptr<Turbopump::Command>& command);
	const std::shared_ptr<Turbopump::Command>& command() const;

	void push(std::string buffer);
	bool pop(std::string& buffer);

protected:
	std::shared_ptr<Turbopump::Command> _command;
	std::queue<std::string> _pending;
};
