/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "UserActionContext.h"

#include "IUserPacketHandler.h"
#include "common/DataBuffer.h"
#include "http/Url.h"

#include <functional>
#include <vector>
using std::string;
using namespace std::placeholders;

UserActionContext::UserActionContext(const IUserPacketHandler& handler)
	: _handler(handler)
{
	_parser.setOnUrl( std::bind(&UserActionContext::onUrl, this, _1, _2) );
	_parser.setOnHeadersComplete( std::bind(&UserActionContext::onBegin, this, _1) );
	_parser.setOnBody( std::bind(&UserActionContext::onBody, this, _1, _2) );
	_parser.setOnMessageComplete( std::bind(&UserActionContext::onComplete, this) );
}

bool UserActionContext::feed(const char* buff, unsigned len)
{
	return _parser.parseBuffer(buff, len);
}

int UserActionContext::onUrl(const char* data, size_t len)
{
	if (_action)
		return 1;
	_url.append(string(data, len));
	return 0;
}

int UserActionContext::onBegin(HttpParser::Status status)
{
	Url url(_url);
	std::vector<string> parts = url.components();
	if (!parts.empty())
		_action = _handler.newAction(parts.front(), url.params());
	return 0;
}

int UserActionContext::onBody(const char* data, size_t len)
{
	if (_action && _action->good())
	{
		DataBuffer dbuf(data, len);
		_action->run(dbuf);
	}
	return 0;
}

int UserActionContext::onComplete()
{
	if (_action && _action->good())
		_action->run(DataBuffer::Null());

	_params.clear();
	_url.clear();
	_action.reset();
	return 0;
}
