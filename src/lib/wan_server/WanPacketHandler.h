#pragma once

#include <map>
#include <memory>
#include <string>

class IAction;
class IDataStore;
class IMembership;
class IPeerTracker;
class IIpSocket;

// receiving (UdpSocket&, string& buff),
// 1) negotiate connections as necessary
// 2) delegate and inform the appropriate virtual connection
class WanPacketHandler
{
public:
	WanPacketHandler(const IMembership& membership, IPeerTracker& peers, IDataStore& dataStore);

	bool onPacket(const IIpSocket& socket, const std::string& buffer);

protected:
	std::shared_ptr<IAction> newAction(const std::string& cmdname, const std::map<std::string,std::string>& params);

protected:
	const IMembership& _membership;
	IPeerTracker&  _peers;
	IDataStore& _dataStore;
};
