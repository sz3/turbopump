/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <memory>
#include <string>

class IAction;
class IDataStore;
class IExecutor;
class IIpSocket;
class ILog;
class IMembership;
class IPeerTracker;
class ISynchronize;
class Peer;
class PeerConnection;
class TurboApi;

// receiving (UdpSocket&, string& buff),
// 1) negotiate connections as necessary
// 2) delegate and inform the appropriate virtual connection
class WanPacketHandler
{
public:
	WanPacketHandler(IExecutor& executor, const IMembership& membership, IPeerTracker& peers, IDataStore& dataStore, ISynchronize& sync, ILog& logger, const TurboApi& callbacks);

	bool onPacket(const IIpSocket& socket, const std::string& buffer);
	void doWork(std::weak_ptr<Peer> weakPeer, std::weak_ptr<PeerConnection> weakConn);
	void processPendingBuffers(const Peer& peer, PeerConnection& conn);

protected:
	std::shared_ptr<IAction> newAction(const Peer& peer, const std::string& cmdname, const std::map<std::string,std::string>& params);

protected:
	IExecutor& _executor;
	const IMembership& _membership;
	IPeerTracker&  _peers;
	IDataStore& _dataStore;
	ISynchronize& _sync;
	ILog& _logger;
	const TurboApi& _callbacks;
};
