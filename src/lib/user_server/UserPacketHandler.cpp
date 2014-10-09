/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "UserPacketHandler.h"

#include "UserCommandContext.h"
#include "api/Api.h"
#include "http/IHttpByteStream.h"
#include "http/StatusCode.h"
#include <vector>
using std::string;

UserPacketHandler::UserPacketHandler(IHttpByteStream& stream, Turbopump::Api& api)
	: _stream(stream)
	, _api(api)
{
}

void UserPacketHandler::run()
{
	UserCommandContext context(*this);
	std::vector<char> buff;
	buff.resize(8192);

	while (1)
	{
		int bytesRead = _stream.read(&buff[0], buff.capacity());
		if (bytesRead <= 0)
			break;

		if (!context.feed(buff.data(), bytesRead))
			break;
	}
}

void UserPacketHandler::sendResponse(StatusCode status)
{
	_stream.setStatus(status.integer());
	_stream.write(NULL, 0);
}

std::unique_ptr<Turbopump::Command> UserPacketHandler::command(const string& cmd, const std::unordered_map<string,string>& params) const
{
	std::unique_ptr<Turbopump::Command> command = _api.command(cmd, params);
	command->setWriter(&_stream);
	return command;
}
