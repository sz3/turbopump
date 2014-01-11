/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Callbacks.h"

#include "MirrorToPeer.h"
#include "RandomizedMirrorToPeer.h"
#include "cohesion/IMerkleIndex.h"
#include "common/KeyMetadata.h"
#include <deque>
#include <functional>
using std::bind;
using std::deque;
using std::function;
using namespace std::placeholders;

// TODO: rather than anonymous namespace, should split these functions out somewhere else...
namespace
{
	auto merkleAddFunct(IMerkleIndex& merkleIndex)
	{
		return [&] (KeyMetadata md, IDataStoreReader::ptr contents)
		{
			merkleIndex.add(md.filename);
		};
	}

	auto writeChainFunct_cloneMode(const IMembership& membership, IPeerTracker& peers)
	{
		std::shared_ptr<RandomizedMirrorToPeer> cmd(new RandomizedMirrorToPeer(membership, peers));
		return bind(&RandomizedMirrorToPeer::run, cmd, _1, _2);
	}

	auto writeChainFunct_partitionMode(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers)
	{
		std::shared_ptr<MirrorToPeer> cmd(new MirrorToPeer(ring, membership, peers));
		return bind(&MirrorToPeer::run, cmd, _1, _2);
	}
}

Callbacks::Callbacks()
{
}

Callbacks::Callbacks(const TurboApi& instruct)
	: TurboApi(instruct)
{
}

void Callbacks::initialize(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers, IMerkleIndex& merkleIndex)
{
	// TODO: devise a proper callback strategy for configurable default callbacks + user defined ones.
	//  yes, I know this is basically: "TODO: figure out how to land on moon"

	// on local write
	{
		deque<function<void(KeyMetadata, IDataStoreReader::ptr)>> functionChainer;
		if (TurboApi::options.merkle)
			functionChainer.push_back( merkleAddFunct(merkleIndex) );

		if (when_local_write_finishes)
			functionChainer.push_back(when_local_write_finishes);

		if (TurboApi::options.write_chaining)
		{
			if (TurboApi::options.partition_keys)
				functionChainer.push_back( writeChainFunct_partitionMode(ring, membership, peers) );
			else
				functionChainer.push_back( writeChainFunct_cloneMode(membership, peers) );
		}

		if (!functionChainer.empty())
		{
			when_local_write_finishes = [functionChainer] (KeyMetadata md, IDataStoreReader::ptr contents)
			{
				for (auto fun : functionChainer)
					fun(md, contents);
			};
		}
	}

	// on mirror write
	{
		deque<function<void(KeyMetadata, IDataStoreReader::ptr)>> functionChainer;
		if (TurboApi::options.merkle)
			functionChainer.push_back( merkleAddFunct(merkleIndex) );

		if (when_mirror_write_finishes)
			functionChainer.push_back(when_mirror_write_finishes);

		if (TurboApi::options.write_chaining && TurboApi::options.partition_keys)
			functionChainer.push_back( writeChainFunct_partitionMode(ring, membership, peers) );

		if (!functionChainer.empty())
		{
			when_mirror_write_finishes = [functionChainer] (KeyMetadata md, IDataStoreReader::ptr contents)
			{
				for (auto fun : functionChainer)
					fun(md, contents);
			};
		}
	}
}
