/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Callbacks.h"

#include "MirrorToPeer.h"
#include "NotifyWriteComplete.h"
#include "RandomizedMirrorToPeer.h"

#include "cohesion/IMerkleIndex.h"
#include "common/KeyMetadata.h"
#include "util/FunctionChainer.h"

#include <deque>
#include <functional>
using std::bind;
using std::deque;
using std::function;
using namespace std::placeholders;

// TODO: rather than anonymous namespace, should split these functions out somewhere else...
namespace
{
	std::function<void(KeyMetadata, IDataStoreReader::ptr)> merkleAddFunct(IMerkleIndex& merkleIndex)
	{
		return [&] (KeyMetadata md, IDataStoreReader::ptr contents)
		{
			merkleIndex.add(md.filename);
		};
	}

	std::function<void(KeyMetadata)> merkleDelFunct(IMerkleIndex& merkleIndex)
	{
		return [&] (KeyMetadata md)
		{
			merkleIndex.remove(md.filename);
		};
	}

	std::function<void(KeyMetadata, IDataStoreReader::ptr)> notifyWriteComplete(const IMembership& membership, IMessageSender& messenger)
	{
		std::shared_ptr<NotifyWriteComplete> cmd(new NotifyWriteComplete(membership, messenger));
		return bind(&NotifyWriteComplete::run, cmd, _1, _2);
	}

	std::function<void(KeyMetadata, IDataStoreReader::ptr)> writeChainFunct_cloneMode(const IMembership& membership, IPeerTracker& peers)
	{
		std::shared_ptr<RandomizedMirrorToPeer> cmd(new RandomizedMirrorToPeer(membership, peers));
		return bind(&RandomizedMirrorToPeer::run, cmd, _1, _2);
	}

	std::function<void(KeyMetadata, IDataStoreReader::ptr)> writeChainFunct_partitionMode(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers)
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

void Callbacks::initialize(const IHashRing& ring, const IMembership& membership, IMerkleIndex& merkleIndex, IMessageSender& messenger, IPeerTracker& peers)
{
	// TODO: devise a proper callback strategy for configurable default callbacks + user defined ones.
	//  yes, I know this is basically: "TODO: figure out how to land on moon"

	// on local write
	{
		FunctionChainer<KeyMetadata, IDataStoreReader::ptr> chain(when_local_write_finishes);
		if (TurboApi::options.merkle)
			chain.add( merkleAddFunct(merkleIndex) );

		if (TurboApi::options.write_chaining)
		{
			if (TurboApi::options.partition_keys)
				chain.add( writeChainFunct_partitionMode(ring, membership, peers) );
			else
				chain.add( writeChainFunct_cloneMode(membership, peers) );
		}
		when_local_write_finishes = chain.generate();
	}

	// on mirror write
	{
		FunctionChainer<KeyMetadata, IDataStoreReader::ptr> chain(when_mirror_write_finishes);
		if (TurboApi::options.merkle)
			chain.add( merkleAddFunct(merkleIndex) );

		if (TurboApi::options.write_chaining && TurboApi::options.partition_keys)
			chain.add( writeChainFunct_partitionMode(ring, membership, peers) );

		chain.add( notifyWriteComplete(membership, messenger) );

		when_mirror_write_finishes = chain.generate();
	}

	// on drop
	{
		FunctionChainer<KeyMetadata> chain(when_drop_finishes);
		if (TurboApi::options.merkle)
			chain.add( merkleDelFunct(merkleIndex) );
		when_drop_finishes = chain.generate();
	}
}
