#pragma once

#include "IPeerTracker.h"

#include <string>
#include <unordered_map>
#include "tbb/concurrent_unordered_map.h"
class BufferedConnectionWriter;
class UdpServer;

class PeerTracker : public IPeerTracker
{
public:
	PeerTracker(const UdpServer& server);

	std::unique_ptr<ConnectionWriteStream> getWriter(const Peer& peer);
	std::shared_ptr<PeerConnection> track(const Peer& peer);

	std::string list() const;

protected:
	const UdpServer& _server;
	tbb::concurrent_unordered_map< std::string,std::shared_ptr<BufferedConnectionWriter> > _writers;
	std::unordered_map< std::string,std::shared_ptr<PeerConnection> > _peers;
};
