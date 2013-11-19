#include "WanPacketHandler.h"

#include "IPeerTracker.h"
#include "PeerConnection.h"
#include "actions/MerkleAction.h"
#include "actions/ReadAction.h"
#include "actions/WriteAction.h"

#include "common/ActionParser.h"
#include "common/DataBuffer.h"
#include "data_store/IDataStore.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include "socket/IpAddress.h"
#include "socket/UdpSocket.h"

#include <iostream>
#include <memory>
#include <utility>
using std::string;
using std::shared_ptr;

// TODO: Lots of member objects, ala IDataStore&?
WanPacketHandler::WanPacketHandler(const IMembership& membership, IPeerTracker& peers, IDataStore& dataStore, ISynchronize& sync)
	: _membership(membership)
	, _peers(peers)
	, _dataStore(dataStore)
	, _sync(sync)
{
}

// would like to make this guy completely ambivalent to UDT stream / UDP datagram nonsense.
// if it's a i.e. this guy is always running on a dedicated thread, with a dedicated buffer,
// and he doesn't need to worry about anything of the sort.
// likewise, PeerConnection should encapsulate all queuing/scheduling/etc bullcrap on the response sending side.
bool WanPacketHandler::onPacket(const IIpSocket& socket, const string& buffer)
{
	// is the message from a valid peer?
	std::shared_ptr<Peer> peer = _membership.lookupIp(socket.getTarget().toString());
	if (!peer)
	{
		std::cerr << "rejecting packet from unknown host " << socket.getTarget().toString() << std::endl;
		return false;
	}

	// is the message unmodified and from the peer we think it is? (i.e. does it decrypt?)
	// TODO: interface for encryption + decryption.
	// also, who should manage the allocation of this buffer?
	string decryptedBuffer = buffer;
	// if decryption fails,
	// return false

	// maybe we're the only one that ever uses _peers, e.g. it is single-threaded?
	std::shared_ptr<PeerConnection> conn = _peers.track(*peer);
	if (!conn)
		return false; // TODO: log error or something

	// TODO: this is currently a hackjob. if there's an action, we clobber our existing one.
	ActionParser parser;
	DataBuffer buff(decryptedBuffer.data(), decryptedBuffer.size());
	if (parser.parse(buff))
		conn->setAction( newAction(*peer, parser.action(), parser.params()) );

	if (!conn->action() || !conn->action()->good())
		return false;
	std::cerr << "received packet '" << decryptedBuffer << "' from " << socket.getTarget().toString() << ". Calling " << conn->action()->name() << std::endl;
	conn->action()->run(buff);
	return true;
}

std::shared_ptr<IAction> WanPacketHandler::newAction(const Peer& peer, const string& cmdname, const std::map<string,string>& params)
{
	std::shared_ptr<IAction> action;
	if (cmdname == "write")
		action.reset(new WriteAction(_dataStore));
	else if (cmdname == "merkle")
		action.reset(new MerkleAction(peer, _sync));
	//else if (cmdname == "ip")
		//action.reset(new IpUpdateAction());
	else
		return action;

	action->setParams(params);
	return action;
}
