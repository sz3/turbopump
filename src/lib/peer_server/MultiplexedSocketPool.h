/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "BufferedConnectionWriter.h"
#include "MultiplexedSocketWriter.h"
#include "socket/ISocketPool.h"
#include "socket/SocketWriter.h"
#include "socket/socket_address.h"
#include "libcuckoo/libcuckoo/cuckoohash_map.hh"
#include <memory>

template <typename Socket>
class MultiplexedSocketPool : public ISocketPool<Socket>
{
protected:
	using map_type = cuckoohash_map<socket_address, std::shared_ptr<BufferedConnectionWriter<Socket>>>;

public:
	std::shared_ptr<ISocketWriter> insert(const Socket& sock);
	bool insert(const Socket& sock, std::shared_ptr<ISocketWriter>& writer);

	std::shared_ptr<ISocketWriter> find(const socket_address& addr) const;

	void close(Socket& sock);
	void close_all();

protected:
	map_type _connections;
};

template <typename Socket>
std::shared_ptr<ISocketWriter> MultiplexedSocketPool<Socket>::insert(const Socket& sock)
{
	std::shared_ptr<ISocketWriter> conn;
	insert(sock, conn);
	return conn;
}

template <typename Socket>
bool MultiplexedSocketPool<Socket>::insert(const Socket& sock, std::shared_ptr<ISocketWriter>& writer)
{
	std::shared_ptr<BufferedConnectionWriter<Socket>> conn(new BufferedConnectionWriter<Socket>(sock));
	bool isnew = true;
	auto updator = [&isnew, &conn] (std::shared_ptr<BufferedConnectionWriter<Socket>>& existing) {
		isnew = false;
		conn = existing;
	};
	_connections.upsert(sock.endpoint(), updator, conn);

	writer = std::shared_ptr<ISocketWriter>(new MultiplexedSocketWriter(conn));
	return isnew;
}

template <typename Socket>
std::shared_ptr<ISocketWriter> MultiplexedSocketPool<Socket>::find(const socket_address& addr) const
{
	std::shared_ptr<BufferedConnectionWriter<Socket>> conn;
	if (!_connections.find(addr, conn))
		return NULL;
	// increment here
	return std::shared_ptr<ISocketWriter>(new MultiplexedSocketWriter(conn));
}

template <typename Socket>
void MultiplexedSocketPool<Socket>::close(Socket& sock)
{
	sock.close();
	_connections.erase(sock.endpoint());
}

template <typename Socket>
void MultiplexedSocketPool<Socket>::close_all()
{
	for (typename map_type::iterator conn = _connections.begin(); !conn.is_end(); ++conn)
		if (!!conn->second)
			conn->second->close();
	_connections.clear();
}

