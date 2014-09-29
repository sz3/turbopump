/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IUserPacketHandler.h"

namespace Turbopump { class Api; }
class IHttpByteStream;

// "owns" the stream
class UserPacketHandler : public IUserPacketHandler
{
public:
	UserPacketHandler(IHttpByteStream& stream, Turbopump::Api& api);

	void run();
	void sendResponse(StatusCode status);

	std::unique_ptr<Turbopump::Command> command(const std::string& cmd, const std::unordered_map<std::string,std::string>& params) const;

protected:
	IHttpByteStream& _stream;
	Turbopump::Api& _api;
};

