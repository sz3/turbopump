/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "peer_client/IPartialTransfers.h"
#include "socket/ISocketWriter.h"
#include "util/CallHistory.h"

class MockPartialTransfers : public IPartialTransfers
{
public:
	void add(ISocketWriter& writer, const std::function<bool()>& fun)
	{
		_history.call("add", writer.target());
		_capturedFun = fun;
	}

	bool run(ISocketWriter& writer)
	{
		_history.call("run", writer.target());
		return true;
	}

public:
	CallHistory _history;

	std::function<bool()> _capturedFun;
};
