/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/WriteInstructions.h"
#include "data_store/IDataStoreReader.h"
#include "peer_client/ISuperviseWrites.h"

class ILocateKeys;
class IMembership;
class Peer;

template <typename MirrorStrategy>
class ChainWrite
{
public:
	ChainWrite(const ILocateKeys& locator, const IMembership& membership, ISuperviseWrites& writer, bool blocking)
		: _locator(locator)
		, _membership(membership)
		, _writer(writer)
		, _blocking(blocking)
	{}

	bool run(WriteInstructions& params, IDataStoreReader::ptr contents)
	{
		if (!params.outstream)
		{
			MirrorStrategy strategy(_locator, _membership);
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
	const ILocateKeys& _locator;
	const IMembership& _membership;
	ISuperviseWrites& _writer;
	bool _blocking;
};
