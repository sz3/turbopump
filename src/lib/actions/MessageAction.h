#pragma once

#include "IAction.h"

class IByteStream;

class MessageAction : public IAction
{
public:
	MessageAction(const std::string& message, IByteStream& writer);

	std::string name() const;
	bool run(const DataBuffer& data);

protected:
	std::string _message;
	IByteStream& _writer;
};

