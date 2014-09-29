/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "UserActionContext.h"

#include "IUserPacketHandler.h"
#include "http/Url.h"

#include <functional>
#include <vector>
using std::string;
using namespace std::placeholders;

UserActionContext::UserActionContext(IUserPacketHandler& handler)
	: _handler(handler)
	, _status(0)
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

StatusCode UserActionContext::status() const
{
	return _status;
}

int UserActionContext::onUrl(const char* data, size_t len)
{
	// we shouldn't have a command at this point.
	// If we do, somebody effed up the protocol (either us or the client) ...
	// return 1 == bail out.
	if (!_command)
	{
		_url.append(string(data, len));
		return 0;
	}
	return 1;
}

int UserActionContext::onBegin(HttpParser::Status status)
{
	Url url(_url);
	std::vector<string> parts = url.components();
	if (!parts.empty())
		_command = _handler.command(parts.front(), url.params());
	if (!_command)
		_status = StatusCode::BadRequest;
	return 0;
}

int UserActionContext::onBody(const char* data, size_t len)
{
	// that is, if the command is bad, just eat the body and do nothing w/ it
	// we'll be complaining back to the client momentarily. :)
	if (!!_command && _status == 0)
	{
		bool res = _command->run(data, len);
		_status = _command->status();
		if (_status == 0)
			_status = res? StatusCode::Success : StatusCode::InternalServerError;
	}
	return 0;
}

int UserActionContext::onComplete()
{
	if (!!_command && _status == 0)
	{
		bool res = _command->run();
		_status = _command->status();
		if (_status == 0)
			_status = res? StatusCode::Success : StatusCode::InternalServerError;
	}

	if (_status == 0)
		_status = StatusCode::NotAcceptable;
	_handler.sendResponse(_status);

	_params.clear();
	_url.clear();
	_command.reset();
	_status = 0;
	return 0;
}
