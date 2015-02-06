/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "PartialTransfers.h"

#include "socket/ISocketServer.h"
using mutex_lock = std::lock_guard<std::mutex>;

PartialTransfers::PartialTransfers(ISocketServer& server)
	: _server(server)
{
}

void PartialTransfers::add(int id, const std::function<bool()>& fun)
{
	{
		mutex_lock lock(_mutex);
		_transfers[id].push_back(fun);
	}
	_server.waitForWriter(id);
}

bool PartialTransfers::empty(int id) const
{
	mutex_lock lock(_mutex);
	auto it = _transfers.find(id);
	return it == _transfers.end() || it->second.empty();
}

// true == done (for now)
// false == more work to do
bool PartialTransfers::run(int id)
{
	while ( !empty(id) )
	{
		std::function<bool()> fun;
		{
			mutex_lock lock(_mutex);
			fun = _transfers[id].front();
			_transfers[id].pop_front();
		}
		if ( !fun() )
			return false;
	}

	mutex_lock lock(_mutex);
	auto it = _transfers.find(id);
	if (it == _transfers.end())
		return true;
	else if (it->second.empty())
	{
		_transfers.erase(it);
		return true;
	}
	return false;
}
