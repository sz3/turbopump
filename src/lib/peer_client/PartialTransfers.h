/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IPartialTransfers.h"
#include <deque>
#include <map>
#include <mutex>

class ISocketServer;

class PartialTransfers : public IPartialTransfers
{
public:
	PartialTransfers(ISocketServer& server);

	void add(ISocketWriter& writer, const std::function<bool()>& fun);
	bool run(ISocketWriter& writer);

protected:
	bool empty(uintptr_t id) const;
	bool empty_unlocked(uintptr_t id) const;

protected:
	ISocketServer& _server;

	mutable std::mutex _mutex;
	std::map< uintptr_t, std::deque<std::function<bool()>> > _transfers;
};
