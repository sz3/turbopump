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
