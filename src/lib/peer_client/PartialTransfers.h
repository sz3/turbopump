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

	void add(int id, const std::function<bool()>& fun);
	bool run(int id);

protected:
	bool empty(int id) const;

protected:
	ISocketServer& _server;

	mutable std::mutex _mutex;
	std::map< int, std::deque<std::function<bool()>> > _transfers;
};
