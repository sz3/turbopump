/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MessageSender.h"

#include "IMessagePacker.h"
#include "api/AckWrite.h"
#include "api/DemandWrite.h"
#include "api/HealKey.h"
#include "api/KeyRequest.h"
#include "api/OfferWrite.h"
#include "api/Sync.h"
#include "common/MerklePoint.h"
#include "deskew/TreeId.h"
#include "membership/Peer.h"
#include "peer_server/IBufferedConnectionWriter.h"
#include "peer_server/IPeerTracker.h"

#include "msgpack.hpp"
#include <memory>
#include <sstream>
using std::shared_ptr;
using std::string;

// TODO: class.
namespace {
	template <typename Req>
	string reqHeader(const IMessagePacker& packer, const Req& request)
	{
		msgpack::sbuffer sbuf;
		msgpack::pack(&sbuf, request);
		return packer.package(request._ID, sbuf.data(), sbuf.size());
	}
}

MessageSender::MessageSender(const IMessagePacker& packer, IPeerTracker& peers)
	: _packer(packer)
	, _peers(peers)
{
}

bool MessageSender::sendMessage(const Peer& peer, const string& message, bool blocking)
{
	shared_ptr<IBufferedConnectionWriter> writer(_peers.getWriter(peer));
	if (!writer)
		return false;

	writer->write(peer.nextActionId(), message.data(), message.size(), blocking);
	writer->flush(blocking);
	return true;
}

void MessageSender::digestPing(const Peer& peer, const TreeId& treeid, const MerklePoint& point)
{
	Turbopump::Sync req;
	req.id = treeid.id;
	req.mirrors = treeid.mirrors;
	sendMessage(peer, reqHeader(_packer, req) + MerklePointSerializer::toString(point));
}

void MessageSender::digestPing(const Peer& peer, const TreeId& treeid, const std::deque<MerklePoint>& points)
{
	Turbopump::Sync req;
	req.id = treeid.id;
	req.mirrors = treeid.mirrors;

	std::stringstream msg;
	msg << MerklePointSerializer::toString(points.front());
	for (auto it = ++points.begin(); it != points.end(); ++it)
		msg << "|" << MerklePointSerializer::toString(*it);
	sendMessage(peer, reqHeader(_packer, req) + msg.str());
}

void MessageSender::requestKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last)
{
	Turbopump::KeyRequest req;
	req.id = treeid.id;
	req.mirrors = treeid.mirrors;
	req.first = first;
	req.last = last;
	sendMessage(peer, reqHeader(_packer, req));
}

void MessageSender::requestHealKey(const Peer& peer, const TreeId& treeid, unsigned long long key)
{
	Turbopump::HealKey req;
	req.id = treeid.id;
	req.mirrors = treeid.mirrors;
	req.key = key;
	sendMessage(peer, reqHeader(_packer, req));
}

void MessageSender::offerWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source)
{
	Turbopump::OfferWrite req;
	req.name = filename;
	req.version = version;
	req.source = source;
	sendMessage(peer, reqHeader(_packer, req));
}

void MessageSender::demandWrite(const Peer& peer, const std::string& filename, const std::string& version, const std::string& source)
{
	Turbopump::DemandWrite req;
	req.name = filename;
	req.version = version;
	req.source = source;
	sendMessage(peer, reqHeader(_packer, req));
}

// TODO: since this could block, may need to move write acks to their own thread.
// or do a two stage "try -> fail -> schedule retry" thing.
void MessageSender::acknowledgeWrite(const Peer& peer, const string& filename, const std::string& version, unsigned long long size)
{
	Turbopump::AckWrite req;
	req.name = filename;
	req.version = version;
	req.size = size;
	sendMessage(peer, reqHeader(_packer, req), true);
}
