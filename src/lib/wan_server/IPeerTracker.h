#pragma once

#include <memory>

class Peer;
class PeerConnection;

class IPeerTracker
{
public:
	virtual ~IPeerTracker() {}
	virtual std::shared_ptr<PeerConnection> track(const Peer& peer) = 0;
	virtual bool decode(const Peer& peer, const std::string& encoded, std::shared_ptr<PeerConnection>& conn, std::string& decoded) = 0;
};
