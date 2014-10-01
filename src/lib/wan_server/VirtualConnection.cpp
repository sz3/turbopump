/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "VirtualConnection.h"

void VirtualConnection::setCommand(const std::shared_ptr<Turbopump::Command>& command)
{
	_command = command;
}

const std::shared_ptr<Turbopump::Command>& VirtualConnection::command() const
{
	return _command;
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
