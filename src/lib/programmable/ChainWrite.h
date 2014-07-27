/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "actions/WriteParams.h"
#include "actions_req/WriteActionSender.h"
#include "data_store/IDataStoreReader.h"

class IHashRing;
class IMembership;

template <typename MirrorStrategy>
class ChainWrite
{
public:
	ChainWrite(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers, bool blocking)
		: _ring(ring)
		, _membership(membership)
		, _peers(peers)
		, _blocking(blocking)
	{}

	bool run(WriteParams& params, IDataStoreReader::ptr contents)
	{
		WriteActionSender client(_peers, _blocking);
		if (!params.outstream)
		{
			MirrorStrategy strategy(_ring, _membership);
			std::shared_ptr<Peer> peer;
			if (!strategy.chooseMirror(params, peer))
				return false;
			params.outstream = client.open(*peer, params);
			if (!params.outstream)
				return false;
		}
		return client.store(*params.outstream, params, contents);
	}

protected:
	const IHashRing& _ring;
	const IMembership& _membership;
	IPeerTracker& _peers;
	bool _blocking;
};
