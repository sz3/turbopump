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
	std::string prettyBuffer;
	for (unsigned i = 0; i < size; ++i)
	{
		if (buffer[i] < 32)
			prettyBuffer += "{" + StringUtil::str((unsigned)(buffer[i])) + "}";
		else
			prettyBuffer += buffer[i];
	}
	_history.call("send", prettyBuffer);
	return size;
}

int MockIpSocket::recv(std::string& buffer)
{
	_history.call("recv");
	buffer = _recvBuff;
	return buffer.size();
}
