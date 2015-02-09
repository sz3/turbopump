/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "BufferedConnectionWriter.h"
#include "MultiplexedSocketWriter.h"
#include "socket/ISocketPool.h"
#include "socket/SocketWriter.h"
#include "socket/socket_address.h"
#include "tbb/concurrent_unordered_map.h"
#include <memory>

template <typename Socket>
class MultiplexedSocketPool : public ISocketPool<Socket>
{
protected:
	using map_type = tbb::concurrent_unordered_map<std::string,std::shared_ptr<BufferedConnectionWriter<Socket>>>; // concurrent_hash_map?

public:
	bool add(const Socket& sock);
	bool add(const Socket& sock, std::shared_ptr<ISocketWriter>& writer);

	std::shared_ptr<ISocketWriter> find(const socket_address& addr) const;
	std::shared_ptr<ISocketWriter> find_or_add(const Socket& sock);

	void close(Socket& sock);
	void close_all();

protected:
	typename map_type::const_iterator _connections_find(const socket_address& addr) const;
	typename map_type::iterator _connections_find(const socket_address& addr);

protected:
	map_type _connections;
};

template <typename Socket>
bool MultiplexedSocketPool<Socket>::add(const Socket& sock)
{
	std::shared_ptr<ISocketWriter> writer;
	return add(sock, writer);
}

template <typename Socket>
bool MultiplexedSocketPool<Socket>::add(const Socket& sock, std::shared_ptr<ISocketWriter>& writer)
{
	bool isnew = false;
	std::pair< typename map_type::iterator, bool> pear = _connections.insert( {sock.endpoint().toString(), NULL} );
	if (!pear.first->second)
	{
		pear.first->second.reset(new BufferedConnectionWriter<Socket>(sock));
		isnew = true;
	}
	// increment here
	writer = std::shared_ptr<ISocketWriter>(new MultiplexedSocketWriter(pear.first->second));
	return isnew;
}

template <typename Socket>
typename MultiplexedSocketPool<Socket>::map_type::const_iterator MultiplexedSocketPool<Socket>::_connections_find(const socket_address& addr) const
{
	return _connections.find(addr.toString());
}

template <typename Socket>
typename MultiplexedSocketPool<Socket>::map_type::iterator MultiplexedSocketPool<Socket>::_connections_find(const socket_address& addr)
{
	return _connections.find(addr.toString());
}

template <typename Socket>
std::shared_ptr<ISocketWriter> MultiplexedSocketPool<Socket>::find(const socket_address& addr) const
{
	typename map_type::const_iterator it = _connections_find(addr);
	if (it == _connections.end())
		return NULL;
	// increment here
	return std::shared_ptr<ISocketWriter>(new MultiplexedSocketWriter(it->second));
}

template <typename Socket>
std::shared_ptr<ISocketWriter> MultiplexedSocketPool<Socket>::find_or_add(const Socket& sock)
{
	typename map_type::iterator it = _connections_find(sock.endpoint());
	if (it == _connections.end())
	{
		std::shared_ptr<ISocketWriter> res;
		add(sock, res); // TODO: this is... bad?
		return res;
	}
	return std::shared_ptr<ISocketWriter>(new MultiplexedSocketWriter(it->second));
}

template <typename Socket>
void MultiplexedSocketPool<Socket>::close(Socket& sock)
{
	sock.close();
	typename map_type::iterator it = _connections_find(sock.endpoint());
	if (it != _connections.end())
		_connections.unsafe_erase(it);
}

template <typename Socket>
void MultiplexedSocketPool<Socket>::close_all()
{
	for (typename map_type::iterator conn = _connections.begin(); conn != _connections.end(); ++conn)
		if (!!conn->second)
			conn->second->close();
	_connections.clear();
}

