#pragma once

#include <atomic>
#include <string>
#include <vector>

class Peer
{
public:
	Peer(const std::string& uid);
	Peer(const Peer& peer);

	std::string address() const;
	unsigned char nextActionId() const;

	std::string toString() const;
	bool fromString(const std::string& serial);

public:
	std::string uid;
	std::vector<std::string> ips;

protected:
	mutable std::atomic<unsigned char> _actionId;
};
