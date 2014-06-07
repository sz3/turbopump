/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockIpSocket.h"

#include "serialize/StringUtil.h"

MockIpSocket::MockIpSocket()
	: _trySendError(false)
	, _trySendErrorBytes(-1)
{
}

IpAddress MockIpSocket::getTarget() const
{
	_history.call("getTarget");
	return _target;
}

std::string MockIpSocket::destination() const
{
	_history.call("destination");
	return _target.ip();
}

int MockIpSocket::try_send(const char* buffer, unsigned size) const
{
	_history.call("try_send", std::string(buffer, size));
	if (_trySendError)
		return _trySendErrorBytes;
	return size;
}

int MockIpSocket::send(const char* buffer, unsigned size) const
{
	_history.call("send", std::string(buffer, size));
	return size;
}

int MockIpSocket::recv(std::string& buffer)
{
	_history.call("recv");
	buffer = _recvBuff;
	return buffer.size();
}
