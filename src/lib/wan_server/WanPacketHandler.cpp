#include "WanPacketHandler.h"

#include "IPeerTracker.h"
#include "PacketParser.h"
#include "PeerConnection.h"
#include "VirtualConnection.h"
#include "actions/KeyReqAction.h"
#include "actions/MerkleAction.h"
#include "actions/ReadAction.h"
#include "actions/WriteAction.h"

#include "common/ActionParser.h"
#include "common/DataBuffer.h"
#include "data_store/IDataStore.h"
#include "event/IExecutor.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include "programmable/TurboApi.h"
#include "socket/IpAddress.h"
#include "socket/UdpSocket.h"

#include <iostream>
#include <memory>
#include <thread>
#include <utility>
using std::string;
using std::shared_ptr;

// TODO: Lots of member objects, ala IDataStore&?
WanPacketHandler::WanPacketHandler(IExecutor& executor, const IMembership& membership, IPeerTracker& peers, IDataStore& dataStore, ISynchronize& sync, const TurboApi& callbacks)
	: _executor(executor)
	, _membership(membership)
	, _peers(peers)
	, _dataStore(dataStore)
	, _sync(sync)
	, _callbacks(callbacks)
{
}

// would like to make this guy completely ambivalent to UDT stream / UDP datagram nonsense.
// if it's a i.e. this guy is always running on a dedicated thread, with a dedicated buffer,
// and he doesn't need to worry about anything of the sort.
// likewise, PeerConnection should encapsulate all queuing/scheduling/etc bullcrap on the response sending side.
bool WanPacketHandler::onPacket(const IIpSocket& socket, const string& buffer)
{
	// is the message from a valid peer?
	std::shared_ptr<Peer> peer = _membership.lookupIp(socket.getTarget().ip());
	if (!peer)
	{
		std::cerr << "rejecting packet from unknown host " << socket.getTarget().toString() << std::endl;
		return false;
	}

	// is the message unmodified and from the peer we think it is? (i.e. does it decrypt?)
	// TODO: interface for encryption + decryption.
	// We need three components to decrypt:
	//  1) the peer's encryption key (public), which we have in hand
	//  2) our own private encryption key, which the encryption interface ought to have access to (secure RAM?)
	//  3) the nonce, which should be randomly generated as the first 24 bits (3 bytes) of the incoming buffer.
	if (buffer.empty())
		return false;
	// if decryption fails,
	// return false

	string decryptedBuffer = buffer;

	//OrderedPacket packet;
	//packet.seqnum = buffer[0]; // some part of the nonce that only increments on data channel sends?
	//packet.buffer = buffer.substr(1); // decrypted buffer

	// maybe we're the only one that ever uses _peers, e.g. it is single-threaded?
	std::shared_ptr<PeerConnection> conn = _peers.track(*peer);
	if (!conn)
		return false; // TODO: log error or something

	conn->pushRecv(std::move(decryptedBuffer));
	if (conn->begin_processing())
		_executor.execute(std::bind(&WanPacketHandler::doWork, this, std::weak_ptr<Peer>(peer), std::weak_ptr<PeerConnection>(conn)));
	return true;
}

// potentially move this elsewhere.
void WanPacketHandler::doWork(std::weak_ptr<Peer> weakPeer, std::weak_ptr<PeerConnection> weakConn)
{
	std::shared_ptr<Peer> peer = weakPeer.lock();
	if (!peer)
		return;
	std::shared_ptr<PeerConnection> conn = weakConn.lock();
	if (!conn)
		return;

	std::cout << "   WanPacketHandler::doWork(" << std::this_thread::get_id() << ") for " << peer->uid << std::endl;

	processPendingBuffers(*peer, *conn);
	conn->end_processing();
}

void WanPacketHandler::processPendingBuffers(const Peer& peer, PeerConnection& conn)
{
	string buffer;
	while (conn.popRecv(buffer))
	{
		DataBuffer unparsed(buffer.data(), buffer.size());
		PacketParser packetGrabber(unparsed);

		DataBuffer buff(DataBuffer::Null());
		while (unparsed.size() > 0)
		{
			unsigned char virtid;
			if (!packetGrabber.getNext(virtid, buff))
			{
				std::cout << "throwing out '" << buffer << "' from " << peer.uid << std::endl;
				break;
			}

			// TODO: implement action teardown, and only attempt to parse the packet if there is no action
			ActionParser parser;
			std::shared_ptr<IAction> action(conn.action(virtid));
			if (!action || !action->good())
			{
				if (parser.parse(buff))
				{
					std::cout << "received action '" << buffer << "' from " << peer.uid << ". virt " << (unsigned)virtid << ", action = " << parser.action() << std::endl;
					action = newAction(peer, parser.action(), parser.params());
					if (action->multiPacket())
					{
						std::cout << "action is multipacket! " << action->name() << std::endl;
						VirtualConnection& virt = conn[virtid];
						virt.setAction(action);

						//cleanup pending virt data
						string pending;
						while (virt.pop(pending))
							action->run( DataBuffer(pending.data(), pending.size()) );
					}
				}
				else
				{
					std::cout << "   connection " << peer.uid << ":" << (unsigned)virtid << " saved packet of size " << buff.size() << std::endl;
					//conn[virtid].push( buff.str() );
					break;
				}
			}
			//std::cout << "received packet '" << buffer << "' from " << peer.uid << ". Calling " << action->name() << std::endl;
			action->run(buff);
		}
	}
}

std::shared_ptr<IAction> WanPacketHandler::newAction(const Peer& peer, const string& cmdname, const std::map<string,string>& params)
{
	std::shared_ptr<IAction> action;
	if (cmdname == "write")
		action.reset(new WriteAction(_dataStore, _callbacks.when_mirror_write_finishes));
	else if (cmdname == "merkle")
		action.reset(new MerkleAction(peer, _sync));
	else if (cmdname == "key-req")
		action.reset(new KeyReqAction(peer, _sync));
	//else if (cmdname == "ip")
		//action.reset(new IpUpdateAction());
	else
		return action;

	action->setParams(params);
	return action;
}
