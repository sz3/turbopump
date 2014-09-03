/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "actions/WriteParams.h"
#include "actions_req/ISuperviseWrites.h"
#include "data_store/IDataStoreReader.h"

class IHashRing;
class IMembership;
class Peer;

template <typename MirrorStrategy>
class ChainWrite
{
public:
	ChainWrite(const IHashRing& ring, const IMembership& membership, ISuperviseWrites& writer, bool blocking)
		: _ring(ring)
		, _membership(membership)
		, _writer(writer)
		, _blocking(blocking)
	{}

	bool run(WriteParams& params, IDataStoreReader::ptr contents)
	{
		if (!params.outstream)
		{
			MirrorStrategy strategy(_ring, _membership);
			std::shared_ptr<Peer> peer;
			if (!strategy.chooseMirror(params, peer))
				return false;
			params.outstream = _writer.open(*peer, params, _blocking);
			if (!params.outstream)
				return false;
		}
		return _writer.store(*params.outstream, params, contents);
	}

protected:
	const IHashRing& _ring;
	const IMembership& _membership;
	ISuperviseWrites& _writer;
	bool _blocking;
};
