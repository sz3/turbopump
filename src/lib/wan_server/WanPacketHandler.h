/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <memory>
#include <string>

namespace Turbopump { class Api; }
class IExecutor;
class ILog;
class IMembership;
class IPeerTracker;
class ISocketWriter;
class Peer;
class PeerConnection;

// receiving (UdpSocket&, string& buff),
// 1) negotiate connections as necessary
// 2) delegate and inform the appropriate virtual connection
class WanPacketHandler
{
public:
	WanPacketHandler(Turbopump::Api& api, IExecutor& executor, const IMembership& membership, IPeerTracker& peers, ILog& logger);

	bool onPacket(ISocketWriter& writer, const char* buff, unsigned size);
	void doWork(std::weak_ptr<Peer> weakPeer, std::weak_ptr<PeerConnection> weakConn);
	void processPendingBuffers(const std::shared_ptr<Peer>& peer, PeerConnection& conn);

protected:
	Turbopump::Api& _api;
	IExecutor& _executor;
	const IMembership& _membership;
	IPeerTracker&  _peers;
	ILog& _logger;
};
