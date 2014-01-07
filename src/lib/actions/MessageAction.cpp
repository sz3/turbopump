/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MessageAction.h"

#include "socket/IByteStream.h"
using std::string;

MessageAction::MessageAction(const string& message, IByteStream& writer)
	: _message(message)
	, _writer(writer)
{
}

std::string MessageAction::name() const
{
	return "message";
}

bool MessageAction::run(const DataBuffer& data)
{
	_writer.write(_message.data(), _message.size());
	return true;
}
