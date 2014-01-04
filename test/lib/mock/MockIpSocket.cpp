#include "MockIpSocket.h"

#include "serialize/StringUtil.h"

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
