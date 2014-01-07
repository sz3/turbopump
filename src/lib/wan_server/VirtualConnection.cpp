/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "VirtualConnection.h"

#include <iostream>

void VirtualConnection::setAction(const std::shared_ptr<IAction>& action)
{
	_action = action;
}

const std::shared_ptr<IAction>& VirtualConnection::action() const
{
	return _action;
}

void VirtualConnection::push(std::string buffer)
{
	_pending.push(buffer);
}

bool VirtualConnection::pop(std::string& buffer)
{
	if (_pending.empty())
		return false;
	buffer = _pending.front();
	_pending.pop();
	return true;
}
