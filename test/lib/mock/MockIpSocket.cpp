#include "MockIpSocket.h"

#include "serialize/StringUtil.h"

bool MockIpSocket::setTarget(const IpAddress& address)
{
	_history.call("setTarget", address.toString());
	_target = address;
	return true;
}

IpAddress MockIpSocket::getTarget() const
{
	_history.call("getTarget");
	return _target;
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
