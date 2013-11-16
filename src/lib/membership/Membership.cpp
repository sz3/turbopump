#include "Membership.h"

#include "Peer.h"
#include "file/StateSaver.h"
#include "serialize/StringUtil.h"
#include "util/Random.h"
#include <utility>
using std::map;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using namespace std::placeholders;

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

Membership::Membership(const std::string& filename, const std::string& myip)
	: _filename(filename)
	, _me(myip)
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
			_ips[*it] = mem;
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
	it->second->ips.push_back(ip);
	_ips[ip] = it->second;
}

shared_ptr<Peer> Membership::lookupIp(const std::string& ip) const
{
	unordered_map< string,shared_ptr<Peer> >::const_iterator it = _ips.find(ip);
	if (it == _ips.end())
		return NULL;
	return it->second;
}

// right now, we track our identity by using the port passed into TurboPumpApp.
// once we have uids working properly, we'll use that and it'll all make a lot more sense.
shared_ptr<Peer> Membership::self() const
{
	return lookupIp(_me);
}

shared_ptr<Peer> Membership::randomPeer() const
{
	unordered_map< string,shared_ptr<Peer> >::const_iterator it = Random::select(_ips.begin(), _ips.end(), _ips.size());
	if (it == _ips.end())
		return NULL;
	return it->second;
}

std::string Membership::toString() const
{
	return StringUtil::stlJoin(_members, '\n');
}
