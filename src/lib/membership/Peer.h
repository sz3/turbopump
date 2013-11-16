#pragma once

#include <string>
#include <vector>

struct Peer
{
	Peer(const std::string& uid);

	std::string address() const;

	std::string toString() const;
	bool fromString(const std::string& serial);

	std::string uid;
	std::vector<std::string> ips;
};
