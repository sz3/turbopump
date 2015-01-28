/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "PartialTransfers.h"

#include "socket/ISocketServer.h"
using mutex_lock = std::lock_guard<std::mutex>;

PartialTransfers::PartialTransfers(ISocketServer& server)
	: _server(server)
{
}

void PartialTransfers::add(ISocketWriter& writer, const std::function<bool()>& fun)
{
	{
		mutex_lock lock(_mutex);
		_transfers[reinterpret_cast<uintptr_t>(&writer)].push_back(fun);
	}
	_server.waitForWriter(writer);
}

bool PartialTransfers::empty(uintptr_t id) const
{
	mutex_lock lock(_mutex);
	auto it = _transfers.find(id);
	return it == _transfers.end() || it->second.empty();
}

// true == done (for now)
// false == more work to do
bool PartialTransfers::run(ISocketWriter& writer)
{
	// mutex plz
	uintptr_t id = reinterpret_cast<uintptr_t>(&writer);
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
