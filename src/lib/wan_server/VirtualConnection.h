/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "actions/IAction.h"
#include <queue>
#include <memory>
#include <string>

// TODO: name this better. It's more an action helper. Buffer. Thing.
class VirtualConnection
{
public:
	void setAction(const std::shared_ptr<IAction>& action);
	const std::shared_ptr<IAction>& action() const;

	void push(std::string buffer);
	bool pop(std::string& buffer);

protected:
	std::shared_ptr<IAction> _action;
	std::queue<std::string> _pending;
};
