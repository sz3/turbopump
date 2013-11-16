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
