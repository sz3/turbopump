/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "peer_client/IPartialTransfers.h"
#include "util/CallHistory.h"

class MockPartialTransfers : public IPartialTransfers
{
public:
	void add(int id, const std::function<bool()>& fun)
	{
		_history.call("add", id);
		_capturedFun = fun;
	}

	bool run(int id)
	{
		_history.call("run", id);
		return true;
	}

public:
	CallHistory _history;

	std::function<bool()> _capturedFun;
};
