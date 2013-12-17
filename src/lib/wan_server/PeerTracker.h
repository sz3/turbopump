#pragma once

#include "IPeerTracker.h"

#include <string>
#include <unordered_map>
#include "tbb/concurrent_unordered_map.h"
class IBufferedConnectionWriter;
class IPacketServer;

class PeerTracker : public IPeerTracker
{
public:
	PeerTracker(IPacketServer& server);

	std::shared_ptr<IBufferedConnectionWriter> getWriter(const Peer& peer);
	std::shared_ptr<PeerConnection> track(const Peer& peer);

	std::string list() const;

protected:
	IPacketServer& _server;
	tbb::concurrent_unordered_map< std::string,std::shared_ptr<IBufferedConnectionWriter> > _writers;
	std::unordered_map< std::string,std::shared_ptr<PeerConnection> > _peers;
};
