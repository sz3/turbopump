/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockUserPacketHandler.h"

#include "api/Command.h"
#include "serialize/str_join.h"

namespace {
	template <class T1, class T2>
	std::ostream& operator<<(std::ostream& outstream, const std::pair<T1,T2>& pear)
	{
		outstream << pear.first << "=" << pear.second;
		return outstream;
	}
}

MockUserPacketHandler::MockUserPacketHandler()
	: _command(NULL)
{}

void MockUserPacketHandler::sendResponse(StatusCode status)
{
	_history.call("sendResponse", status.str());
}

std::unique_ptr<Turbopump::Command> MockUserPacketHandler::command(const std::string& cmd, const std::unordered_map<std::string,std::string>& params) const
{
	_history.call("command", cmd, turbo::str::join(params));
	std::unique_ptr<Turbopump::Command> command(_command);
	_command = NULL;
	return command;
}
