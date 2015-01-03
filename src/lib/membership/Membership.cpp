/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Membership.h"

#include "Peer.h"
#include "common/turbopump_defaults.h"
#include "common/KeyMetadata.h"
#include "common/MyMemberId.h"
#include "storage/IStore.h"
#include "storage/writestream.h"

#include "serialize/str.h"
#include "socket/socket_address.h"
#include "util/Random.h"
#include <algorithm>
#include <utility>
using std::map;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using turbo::str::split;
using namespace std::placeholders;

// membership will eventually consist of:
//  * permanent worker ID => used to sign record. Also basis of the hash ring.
//  * ip:port (list)
//  * public key for encryption (will cycle on reboots, or on a time interval to stop the sequence number from climbing too high)

// "How do I talk to a worker ID?" => check membership for his (ed25519 signed) ip:port pair
// "How do I find the worker ID from a socket?" => look at membership.
// "How do I find the worker ID from a fictional (raw) socket?" => PeerTracker should track these somehow?

namespace {
	std::ostream& operator<<(std::ostream& outstream, const std::pair< string,shared_ptr<Peer> >& mem)
	{
		outstream << mem.second->toString();
		return outstream;
	}
}
#include "file/StateSaver.h"
#include "serialize/str_join.h"

Membership::Membership(const std::string& filename, const std::string& myip)
	: _filename(filename)
	, _myip(myip)
{
}

void Membership::loadLine(const std::string& line)
{
	Peer member("");
	if (member.fromString(line))
	{
		std::shared_ptr<Peer> mem(new Peer(member));
		_members[member.uid] = mem;
		for (std::vector<string>::const_iterator it = member.ips.begin(); it != member.ips.end(); ++it)
		{
			if (*it != _myip)
			{
				_ips[*it] = mem;
				std::vector<string> splits = split(*it, ':');
				if (!splits.empty())
					_ips[splits.front()] = mem;
			}
			else
				setSelf(mem);
		}
	}
}

bool Membership::load()
{
	StateSaver saver(_filename);
	return saver.load(std::bind(&Membership::loadLine, this, _1));
}

bool Membership::save()
{
	StateSaver saver(_filename);
	return saver.save(_members.begin(), _members.end());
}

bool Membership::add(const std::string& uid)
{
	std::pair<map< string,shared_ptr<Peer> >::iterator,bool> pear = _members.insert( std::pair< string,shared_ptr<Peer> >(uid, NULL) );
	if (pear.second)
		pear.first->second.reset( new Peer(uid) );
	return pear.second;
}

bool Membership::remove(const std::string& uid)
{
	return _members.erase(uid) > 0;
}

bool Membership::addIp(const std::string& ip, const std::string& uid)
{
	map< string,shared_ptr<Peer> >::iterator it = _members.find(uid);
	if (it == _members.end())
		return false;

	shared_ptr<Peer>& peer = it->second;
	if (std::find(peer->ips.begin(), peer->ips.end(), ip) == peer->ips.end())
	{
		peer->ips.push_back(ip);
		_ips[ip] = it->second;
	}
	return true;
}

shared_ptr<Peer> Membership::lookup(const std::string& uid) const
{
	map< string,shared_ptr<Peer> >::const_iterator it = _members.find(uid);
	if (it == _members.end())
		return NULL;
	return it->second;
}

shared_ptr<Peer> Membership::lookupIp(const std::string& ip) const
{
	unordered_map< string,shared_ptr<Peer> >::const_iterator it = _ips.find(ip);
	if (it == _ips.end())
		return NULL;
	return it->second;
}

// TODO: randomly generate id
bool Membership::addSelf()
{
	if (!!_self)
		return false;

	std::vector<string> splits = split(_myip, ':');
	if (splits.empty())
		return false;

	std::shared_ptr<Peer> me = lookupIp(splits.front());
	if (!me)
	{
		me.reset(new Peer(splits.back()));
		me->ips.push_back(_myip);
		_members[me->uid] = me;
		_ips[splits.front()] = me;
	}
	setSelf(me);
	return true;
}

void Membership::setSelf(shared_ptr<Peer> self)
{
	_self = self;
	MyMemberId(self->uid);
}

shared_ptr<Peer> Membership::self() const
{
	return _self;
}

bool Membership::containsSelf(const std::vector<std::string>& list) const
{
	return std::find(list.begin(), list.end(), _self->uid) != list.end();
}

shared_ptr<Peer> Membership::randomPeer() const
{
	unordered_map< string,shared_ptr<Peer> >::const_iterator it = Random::select(_ips.begin(), _ips.end(), _ips.size());
	if (it == _ips.end())
		return NULL;
	return it->second;
}

shared_ptr<Peer> Membership::randomPeerFromList(std::vector<string> list) const
{
	while (!list.empty())
	{
		std::vector<string>::iterator it = Random::select(list.begin(), list.end(), list.size());
		shared_ptr<Peer> peer = lookup(*it);
		if (peer && peer != self())
			return peer;

		list.erase(it);
	}
	return NULL;
}

void Membership::forEachPeer(std::function<void(const Peer&)> fun) const
{
	for (map<string,shared_ptr<Peer>>::const_iterator it = _members.begin(); it != _members.end(); ++it)
		fun(*it->second);
}

void Membership::syncToDataStore(IStore& store) const
{
	// TODO: this doesn't address the KeyTabulator!!
	// crap is gonna be weird as a result.
	auto fun = [&store] (const Peer& peer)
	{
		writestream writer = store.write(MEMBERSHIP_FILE_PREFIX + peer.uid);
		string data = peer.address();
		writer.write(data.data(), data.size());
		writer.commit(true);
	};
	forEachPeer(fun);
}

std::string Membership::toString() const
{
	return turbo::str::join(_members, '\n');
}
