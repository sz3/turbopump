/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "actions/IAction.h"
#include "http/HttpParser.h"
#include <map>
#include <memory>
#include <string>
class IUserPacketHandler;

class UserActionContext
{
public:
	UserActionContext(const IUserPacketHandler& handler);

	bool feed(const char* buff, unsigned len);

	int onUrl(const char* data, size_t len);
	int onBegin(HttpParser::Status status);
	int onBody(const char* data, size_t len);
	int onComplete();

protected:
	const IUserPacketHandler& _handler;
	HttpParser _parser;

	std::string _url;
	std::unique_ptr<IAction> _action;
	std::map<std::string,std::string> _params;
};
