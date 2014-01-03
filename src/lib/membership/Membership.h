#pragma once

#include "IMembership.h"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// ip:port -> guid
// guid -> ip:port
// guid is master key, changes to ip:port are signed by ed25519.

class Peer;

class Membership : public IMembership
{
public:
	Membership(const std::string& filename, const std::string& myip);

	bool load();
	bool save();

	bool add(const std::string& uid);
	bool remove(const std::string& uid);

	bool addIp(const std::string& ip, const std::string& uid);
	std::shared_ptr<Peer> lookup(const std::string& uid) const;
	std::shared_ptr<Peer> lookupIp(const std::string& ip) const;
	std::shared_ptr<Peer> self() const;
	std::shared_ptr<Peer> randomPeer() const;

	std::string toString() const;

protected:
	void loadLine(const std::string& line);

protected:
	std::string _filename;
	std::string _me;
	std::map< std::string,std::shared_ptr<Peer> > _members; // by uid
	std::unordered_map< std::string,std::shared_ptr<Peer> > _ips;     // by ip
};
