#include "PeerConnection.h"

#include "common/DataBuffer.h"
#include "socket/IIpSocket.h"
#include "socket/IpAddress.h"
#include <utility>

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

int PeerConnection::write(const DataBuffer& data)
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
