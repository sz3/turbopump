/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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

