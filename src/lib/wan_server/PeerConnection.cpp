#include "PeerConnection.h"

#include "common/DataBuffer.h"
#include "socket/IIpSocket.h"
#include "socket/IpAddress.h"
#include <utility>

// manage buffers.
/*
 * udp server -> pushes buffer (concurrent_queue) into connection
 * udp server -> atomic notify (check lock, if thread is running it should be held). If thread is not running, schedule work
 * running thread -> reads next buffer
 *
 * ... in this way, we can keep work for a given operation on the same thread.
 */

// TODO: expiry! Seriously, how?
// TODO: virtual connections.
PeerConnection::PeerConnection(const std::shared_ptr<IIpSocket>& sock)
	: _sock(sock)
{
}

IpAddress PeerConnection::peer() const
{
	return _sock->getTarget();
}

int PeerConnection::send(const DataBuffer& data)
{
	// NOTE: maybe some day do slow start / flow control here!
	// would enable us to correlate peers across sockets...
	// for now, use underlying socket (UDT) to do it.
	return _sock->send(data.buffer(), data.size());
}

void PeerConnection::setAction(const std::shared_ptr<IAction>& action)
{
	_action = action;
}

const std::shared_ptr<IAction>& PeerConnection::action() const
{
	return _action;
}
