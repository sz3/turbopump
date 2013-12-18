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

void VirtualConnection::push(OrderedPacket packet)
{
	_pending.push(packet);
}

bool VirtualConnection::pop(OrderedPacket& packet)
{
	if (_pending.empty())
		return false;
	packet = _pending.top();
	_pending.pop();
	return true;
}
