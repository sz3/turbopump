/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockUserPacketHandler.h"

#include "actions/IAction.h"
#include "serialize/StringUtil.h"

namespace {
	template <class T1, class T2>
	std::ostream& operator<<(std::ostream& outstream, const std::pair<T1,T2>& pear)
	{
		outstream << pear.first << "=" << pear.second;
		return outstream;
	}
}

MockUserPacketHandler::MockUserPacketHandler()
	: _action(NULL)
{}

void MockUserPacketHandler::sendResponse(StatusCode status)
{
	_history.call("sendResponse", status.str());
}

std::unique_ptr<IAction> MockUserPacketHandler::newAction(const std::string& actionName, const std::map<std::string,std::string>& params) const
{
	_history.call("newAction", actionName, StringUtil::join(params));
	std::unique_ptr<IAction> action(_action);
	_action = NULL;
	return action;
}
