/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <memory>
#include <string>

class IAction;
class ICorrectSkew;
class IDataStore;
class IExecutor;
class IHashRing;
class ILocateKeys;
class ILog;
class IMembership;
class IMessageSender;
class IPeerTracker;
class ISocketWriter;
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
	WanPacketHandler(IExecutor& executor, ICorrectSkew& corrector, IDataStore& dataStore, const IHashRing& ring, const ILocateKeys& locator,
					 const IMembership& membership, IMessageSender& messenger, IPeerTracker& peers, ISynchronize& sync, ILog& logger, const TurboApi& callbacks);

	bool onPacket(ISocketWriter& writer, const char* buff, unsigned size);
	void doWork(std::weak_ptr<Peer> weakPeer, std::weak_ptr<PeerConnection> weakConn);
	void processPendingBuffers(const Peer& peer, PeerConnection& conn);

protected:
	std::shared_ptr<IAction> newAction(const Peer& peer, const std::string& cmdname, const std::map<std::string,std::string>& params);

protected:
	IExecutor& _executor;
	ICorrectSkew& _corrector;
	IDataStore& _dataStore;
	const IHashRing& _ring;
	const ILocateKeys& _locator;
	const IMembership& _membership;
	IMessageSender& _messenger;
	IPeerTracker&  _peers;
	ISynchronize& _sync;
	ILog& _logger;
	const TurboApi& _callbacks;
};
