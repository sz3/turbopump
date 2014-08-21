/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "WanPacketHandler.h"

#include "IPeerTracker.h"
#include "PacketParser.h"
#include "PeerConnection.h"
#include "VirtualConnection.h"
#include "actions/AckWriteAction.h"
#include "actions/DemandWriteAction.h"
#include "actions/DropAction.h"
#include "actions/HealKeyAction.h"
#include "actions/KeyReqAction.h"
#include "actions/OfferWriteAction.h"
#include "actions/ReadAction.h"
#include "actions/SyncAction.h"
#include "actions/WriteAction.h"

#include "common/ActionParser.h"
#include "common/DataBuffer.h"
#include "data_store/IDataStore.h"
#include "event/IExecutor.h"
#include "logging/ILog.h"
#include "membership/IMembership.h"
#include "membership/Peer.h"
#include "programmable/TurboApi.h"
#include "serialize/StringUtil.h"
#include "socket/ISocketWriter.h"
#include "socket/IpAddress.h"

#include <iostream>
#include <memory>
#include <thread>
#include <utility>
using std::string;
using std::shared_ptr;

WanPacketHandler::WanPacketHandler(IExecutor& executor, ICorrectSkew& corrector, IDataStore& dataStore, const IHashRing& ring, const ILocateKeys& locator,
								   const IMembership& membership, IMessageSender& messenger, IPeerTracker& peers, ISynchronize& sync,
								   ILog& logger, const TurboApi& callbacks)
	: _executor(executor)
	, _corrector(corrector)
	, _dataStore(dataStore)
	, _ring(ring)
	, _locator(locator)
	, _membership(membership)
	, _messenger(messenger)
	, _peers(peers)
	, _sync(sync)
	, _logger(logger)
	, _callbacks(callbacks)
{
}

// would like to make this guy completely ambivalent to UDT stream / UDP datagram nonsense.
// if it's a i.e. this guy is always running on a dedicated thread, with a dedicated buffer,
// and he doesn't need to worry about anything of the sort.
// likewise, PeerConnection should encapsulate all queuing/scheduling/etc bullcrap on the response sending side.
bool WanPacketHandler::onPacket(ISocketWriter& writer, const char* buff, unsigned size)
{
	// is the message from a valid peer?
	std::shared_ptr<Peer> peer = _membership.lookupIp(writer.target());
	if (!peer)
	{
		_logger.logWarn("rejecting packet from unknown host " + writer.endpoint().toString());
		return false;
	}

	// is the message unmodified and from the peer we think it is? (i.e. does it decrypt?)
	// TODO: interface for encryption + decryption.
	// We need three components to decrypt:
	//  1) the peer's encryption key (public), which we have in hand
	//  2) our own private encryption key, which the encryption interface ought to have access to (secure RAM?)
	//  3) the nonce, which should be randomly generated as the first 24 bits (3 bytes) of the incoming buffer.
	if (size == 0)
		return false;
	// if decryption fails,
	// return false

	string decryptedBuffer = std::string(buff, size);

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

	//std::cout << "   WanPacketHandler::doWork(" << std::this_thread::get_id() << ") for " << peer->uid << std::endl;

	do {
		processPendingBuffers(*peer, *conn);
		conn->end_processing();
	}
	while (!conn->empty() && conn->begin_processing());
}

void WanPacketHandler::processPendingBuffers(const Peer& peer, PeerConnection& conn)
{
	_logger.logTrace("beginning processing for " + peer.uid);

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
					_logger.logTrace("received action '" + buffer + "' from " + peer.uid + ". virt " + StringUtil::str((unsigned)virtid) + ", action = " + parser.action());
					action = newAction(peer, parser.action(), parser.params());
					if (!action)
						continue;
					if (action->multiPacket())
					{
						//std::cout << "action is multipacket! " << action->name() << std::endl;
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
			//_logger.logDebug("received packet '" + buff.str() + "' from " + peer.uid + ". Calling " + action->name());
			action->run(buff);
		}
	}
	_logger.logTrace("ending processing for " + peer.uid);
}

std::shared_ptr<IAction> WanPacketHandler::newAction(const Peer& peer, const string& cmdname, const std::map<string,string>& params)
{
	std::shared_ptr<IAction> action;
	if (cmdname == "write")
	{
		_logger.logDebug("peer " + peer.uid + " is sending me a file! " + params.find("name")->second);
		action.reset(new WriteAction(_dataStore, _callbacks.when_mirror_write_finishes));
	}
	else if (cmdname == "ack-write")
		action.reset(new AckWriteAction(_dataStore, _locator, _callbacks.when_drop_finishes));
	else if (cmdname == "drop")
		action.reset(new DropAction(_dataStore, _locator, _callbacks.when_drop_finishes));
	else if (cmdname == "sync")
		action.reset(new SyncAction(peer, _sync));
	else if (cmdname == "key-req")
		action.reset(new KeyReqAction(peer, _sync));
	else if (cmdname == "heal-key")
		action.reset(new HealKeyAction(peer, _corrector));
	else if (cmdname == "offer-write")
		action.reset(new OfferWriteAction(peer, _dataStore, _messenger));
	else if (cmdname == "demand-write")
		action.reset(new DemandWriteAction(peer, _corrector));
	//else if (cmdname == "ip")
		//action.reset(new IpUpdateAction());
	else
		return action;

	action->setParams(params);
	return action;
}
