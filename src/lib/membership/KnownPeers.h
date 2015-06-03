/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IKnowPeers.h"

#include "Peer.h"
#include "kademlia/routing_table.h"
#include "socket/socket_address.h"

#include <deque>
#include <unordered_map>
class IStore;

namespace std {
	template <>
	struct hash<socket_address>
	{
		std::size_t operator()(const socket_address& addr) const
		{
			using std::string;
			return hash<string>()(addr.address()) xor hash<unsigned short>()(addr.port());
		}
	};

	template <>
	struct hash< std::shared_ptr<Peer> >
	{
		using result_type = size_t;

		std::size_t operator()(const std::shared_ptr<Peer>& peer) const
		{
			using std::string;
			return hash<string>()(peer->uid);
		}

		std::size_t operator()(const Peer& peer) const
		{
			using std::string;
			return hash<string>()(peer.uid);
		}

		std::size_t operator()(const std::string& peer_uid) const
		{
			using std::string;
			return hash<string>()(peer_uid);
		}
	};
}

inline bool operator==(const std::shared_ptr<Peer>& p1, const std::string& uid)
{
	return p1->uid == uid;
}

class KnownPeers : public IKnowPeers
{
public:
	KnownPeers(const std::string& filename);

	bool update(const std::string& uid, const std::vector<std::string>& ips={});
	bool remove(const std::string& uid);

	std::shared_ptr<Peer> self() const;
	std::shared_ptr<Peer> lookup(const std::string& uid) const;
	std::shared_ptr<Peer> lookupAddr(const socket_address& addr) const;
	std::shared_ptr<Peer> randomPeer() const;

	void forEachPeer(std::function<void(const Peer&)> fun) const;
	void syncToDataStore(IStore& store) const;
	std::string toString() const;

	bool save();
	bool load();

protected:
	bool addSelf();

protected:
	std::string _filename;
	std::shared_ptr<Peer> _self;
	kademlia::routing_table< std::shared_ptr<Peer>, std::deque > _members;
	std::unordered_map< socket_address,std::shared_ptr<Peer> > _ips;
};

