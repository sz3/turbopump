/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "KnownPeers.h"

#include "common/MyMemberId.h"
#include "common/turbopump_defaults.h"
#include "storage/IStore.h"
#include "storage/writestream.h"

#include "file/StateSaver.h"
#include "serialize/str.h"
#include <algorithm>
using std::shared_ptr;
using std::string;
using std::unordered_map;

KnownPeers::KnownPeers(const std::string& filename)
	: _filename(filename)
{
}

bool KnownPeers::update(const std::string& uid, const std::vector<std::string>& ips)
{
	if (!_self)
	{
		_self.reset(new Peer(uid, ips));
		_members.set_origin(_self);
		return true;
	}
	else if (uid == _self->uid)
	{
		_self->ips = ips;
		return false;
	}

	shared_ptr<Peer> current;
	auto it = _members.find(uid);
	bool isNew = (it == _members.end());
	if (isNew)
	{
		current.reset(new Peer(uid, ips));
		_members.insert(current);
	}
	else
	{
		current = *it;
		current->ips = ips;
	}

	for (const string& ip : ips)
	{
		// if ip:port, store twice
		// once as socket_address(ip, port)
		// against as socket_address(ip, 0)
		socket_address addr;
		if ( !addr.fromString(ip) )
			continue;

		_ips[addr] = current;
		_ips[socket_address(addr.address())] = current;
	}
	return isNew;
}

bool KnownPeers::remove(const std::string& uid)
{
	auto it = _members.find(uid);
	if (it == _members.end())
		return false;

	for (const string& ip : (*it)->ips)
	{
		socket_address addr;
		if ( !addr.fromString(ip) )
			continue;
		_ips.erase(addr);

		auto ipit = _ips.find(socket_address(addr.address()));
		if (ipit != _ips.end() && (*it)->uid == uid)
			_ips.erase(ipit);
	}
	return _members.erase(uid);
}

bool KnownPeers::addSelf()
{
	if (!!_self)
	{
		MyMemberId(_self->uid);
		return false;
	}

	string peerId = MyMemberId::generate();
	_self.reset(new Peer(peerId));
	_members.set_origin(_self);
	return true;
}

shared_ptr<Peer> KnownPeers::self() const
{
	return _self;
}

shared_ptr<Peer> KnownPeers::lookup(const std::string& uid) const
{
	auto it = _members.find(uid);
	if (it == _members.end())
		return NULL;
	return *it;
}

// lookup twice, once by socket_address(ip, port), once by socket_address(ip, 0)
shared_ptr<Peer> KnownPeers::lookupAddr(const socket_address& addr) const
{
	unordered_map< socket_address,shared_ptr<Peer> >::const_iterator it = _ips.find(addr);
	if (it == _ips.end())
	{
		it = _ips.find(socket_address(addr.address()));
		if (it == _ips.end())
			return NULL;
	}
	return it->second;
}

shared_ptr<Peer> KnownPeers::randomPeer() const
{
	auto it = _members.random();
	if (it == _members.end())
		return NULL;
	return *it;
}

void KnownPeers::forEachPeer(std::function<void(const Peer&)> fun) const
{
	if (!!_self)
		fun(*_self);
	for (auto it = _members.begin(); it != _members.end(); ++it)
		fun(**it);
}

void KnownPeers::syncToDataStore(IStore& store) const
{
	auto fun = [&store] (const Peer& peer)
	{
		VectorClock version;
		version.increment(peer.uid);
		writestream writer = store.write(MEMBERSHIP_FILE_PREFIX + peer.uid, version.toString(), 0);
		if (writer)
		{
			string data = peer.address();
			writer.write(data.data(), data.size());
			writer.commit(true);
		}
	};
	forEachPeer(fun);
}

std::string KnownPeers::toString() const
{
	string summary;
	auto fun = [&summary] (const Peer& peer)
	{
		summary += peer.toString() + '\n';
	};

	forEachPeer(fun);
	return summary;
}

bool KnownPeers::load()
{
	// self will always be first line in file
	// TODO!!! integration tests need to each have their own KnownPeers file...
	auto fun = [this] (const std::string& line)
	{
		Peer peer("");
		if (peer.fromString(line))
			this->update(peer.uid, peer.ips);
	};

	StateSaver saver(_filename);
	bool res = saver.load(fun);
	addSelf();
	return res;
}

bool KnownPeers::save()
{
	return File::save(_filename, toString());
}

