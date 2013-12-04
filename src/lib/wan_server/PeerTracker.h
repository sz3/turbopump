#pragma once

#include "IPeerTracker.h"

#include <string>
#include <unordered_map>

class PeerConnection;
class UdpServer;

class PeerTracker : public IPeerTracker
{
public:
	PeerTracker(const UdpServer& server);

	std::shared_ptr<PeerConnection> track(const Peer& peer);

	std::string list() const;

protected:
	const UdpServer& _server;
	std::unordered_map< std::string,std::shared_ptr<PeerConnection> > _peers;
};
