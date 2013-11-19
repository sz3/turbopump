#include "PeerTracker.h"

#include "PeerConnection.h"

#include "membership/Peer.h"
#include "socket/IpAddress.h"
#include "socket/UdpServer.h"
#include <iostream>
#include <sstream>
#include <utility>
using std::string;
using std::unordered_map;

// We've got all kinds of problems in here at the moment.
// 1) map of PCs just grows and grows
// 2) decryptMessage and find make very little sense as belonging to the same object.
// 3) ...

// TODO: rather than making connections age out or something, note that what we want more than an ip address is a *peer ID*.
//  as such, it would be useful to have a definitive member list for lookup of peers. The PeerTracker would encapsulate the peer->ip:port
//  lookups, while PeerConnections would encapsulate the RAM-only ip:port->socket->action part of the equation.
//  In this case, aging out peers is just bounding the size of the map based on some LRU scheme -- it's only useful to have N peers in play at a time.

// TODO: for now, there are no guids. We essentially auto-join every socket as a new member (ip:port) who never leaves.
//  Guid will be a cryptographic public key.

// "How do I talk to a worker ID?" => check membership for his (ed25519 signed) ip:port pair
// "How do I find the worker ID from a socket?" => look at membership.
// "How do I find the worker ID from a fictional (raw) socket?" => PeerTracker should track these somehow?

using peerit = unordered_map<string,std::shared_ptr<PeerConnection>>::iterator;

PeerTracker::PeerTracker(const UdpServer& server)
	: _server(server)
{
}

std::shared_ptr<PeerConnection> PeerTracker::track(const Peer& peer)
{
	IpAddress address;
	if (!address.fromString(peer.address()))
	{
		std::cerr << "BADNESS! Membership has invalid ip address information on peer " << peer.uid << "!!!" << std::endl;
		return NULL;
	}

	std::shared_ptr<IIpSocket> sock(_server.sock());
	sock->setTarget(address);
	std::pair<peerit, bool> pear = _peers.insert( std::pair<string,std::shared_ptr<PeerConnection> >(peer.uid, std::shared_ptr<PeerConnection>(new PeerConnection(sock))) );
	return pear.first->second;
}

bool PeerTracker::decode(const Peer& peer, const std::string& encoded, std::shared_ptr<PeerConnection>& conn, std::string& decoded)
{
	IpAddress address;
	if (!address.fromString(peer.address()))
	{
		std::cerr << "BADNESS! Membership has invalid ip address information on peer " << peer.uid << "!!!" << std::endl;
		return false;
	}

	peerit it = _peers.find(peer.uid);
	if (it == _peers.end())
	{
		// do decryption with sequence number 1
		decoded = encoded;

		// if decryption succeeds, allocate new PeerConnection and insert it
		std::shared_ptr<IIpSocket> sock(_server.sock());
		sock->setTarget(address);

		std::pair<peerit, bool> pear = _peers.insert( std::pair<string,std::shared_ptr<PeerConnection>>(peer.uid, std::shared_ptr<PeerConnection>(new PeerConnection(sock))) );
		conn = pear.first->second;
	}
	else
	{
		conn = it->second;

		// decrypt based on connection's sequence number
		decoded = encoded;
	}
	return true;
}

std::string PeerTracker::list() const
{
	std::stringstream ss;
	for (auto it = _peers.begin(); it != _peers.end(); ++it)
		ss << it->first << " : " << it->second->peer().toString() << std::endl;
	return ss.str();
}
