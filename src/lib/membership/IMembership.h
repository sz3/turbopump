#pragma once

#include <memory>
#include <string>

class Peer;

class IMembership
{
public:
	virtual ~IMembership() {}

	virtual bool addIp(const std::string& ip, const std::string& uid) = 0;
	virtual std::shared_ptr<Peer> lookupIp(const std::string& ip) const = 0;
	virtual std::shared_ptr<Peer> self() const = 0;
	virtual std::shared_ptr<Peer> randomPeer() const = 0;

	virtual std::string toString() const = 0;
};
