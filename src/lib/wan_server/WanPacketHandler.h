#pragma once

#include <map>
#include <memory>
#include <string>

class Callbacks;
class IAction;
class IDataStore;
class IIpSocket;
class IMembership;
class IPeerTracker;
class ISynchronize;
class Peer;

// receiving (UdpSocket&, string& buff),
// 1) negotiate connections as necessary
// 2) delegate and inform the appropriate virtual connection
class WanPacketHandler
{
public:
	WanPacketHandler(const IMembership& membership, IPeerTracker& peers, IDataStore& dataStore, ISynchronize& sync, const Callbacks& callbacks);

	bool onPacket(const IIpSocket& socket, const std::string& buffer);

protected:
	std::shared_ptr<IAction> newAction(const Peer& peer, const std::string& cmdname, const std::map<std::string,std::string>& params);

protected:
	const IMembership& _membership;
	IPeerTracker&  _peers;
	IDataStore& _dataStore;
	ISynchronize& _sync;
	const Callbacks& _callbacks;
};
