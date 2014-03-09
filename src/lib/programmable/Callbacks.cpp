/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Callbacks.h"

#include "AddPeer.h"
#include "MirrorToPeer.h"
#include "NotifyWriteComplete.h"
#include "RandomizedMirrorToPeer.h"

#include "actions/DropParams.h"
#include "actions/WriteParams.h"
#include "cohesion/IKeyTabulator.h"
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
	std::function<void(WriteParams, IDataStoreReader::ptr)> membershipAddFunct(IHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator)
	{
		return [&] (WriteParams params, IDataStoreReader::ptr contents)
		{
			AddPeer adder(ring, membership, keyTabulator);
			adder.run(params, contents);
		};
	}

	std::function<void(WriteParams, IDataStoreReader::ptr)> digestAddFunct(IKeyTabulator& keyTabulator)
	{
		return [&] (WriteParams params, IDataStoreReader::ptr contents)
		{
			keyTabulator.update(params.filename, contents->summary(), params.totalCopies);
		};
	}

	std::function<void(DropParams)> digestDelFunct(IKeyTabulator& keyTabulator)
	{
		return [&] (DropParams params)
		{
			keyTabulator.remove(params.filename, params.totalCopies);
		};
	}

	std::function<void(WriteParams, IDataStoreReader::ptr)> notifyWriteComplete(const IMembership& membership, IMessageSender& messenger)
	{
		std::shared_ptr<NotifyWriteComplete> cmd(new NotifyWriteComplete(membership, messenger));
		return bind(&NotifyWriteComplete::run, cmd, _1, _2);
	}

	std::function<void(WriteParams, IDataStoreReader::ptr)> writeChainFunct_cloneMode(const IMembership& membership, IPeerTracker& peers)
	{
		std::shared_ptr<RandomizedMirrorToPeer> cmd(new RandomizedMirrorToPeer(membership, peers));
		return bind(&RandomizedMirrorToPeer::run, cmd, _1, _2);
	}

	std::function<void(WriteParams, IDataStoreReader::ptr)> writeChainFunct_partitionMode(const IHashRing& ring, const IMembership& membership, IPeerTracker& peers)
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

void Callbacks::initialize(IHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator, IMessageSender& messenger, IPeerTracker& peers)
{
	// TODO: devise a proper callback strategy for configurable default callbacks + user defined ones.
	//  yes, I know this is basically: "TODO: figure out how to land on moon"

	// on local write
	{
		FunctionChainer<WriteParams, IDataStoreReader::ptr> chain(when_local_write_finishes);
		if (TurboApi::options.active_sync)
			chain.add( digestAddFunct(keyTabulator) );

		if (TurboApi::options.write_chaining)
		{
			if (TurboApi::options.partition_keys)
				chain.add( writeChainFunct_partitionMode(ring, membership, peers) );
			else
				chain.add( writeChainFunct_cloneMode(membership, peers) );
		}
		chain.add( membershipAddFunct(ring, membership, keyTabulator) );

		when_local_write_finishes = chain.generate();
	}

	// on mirror write
	{
		FunctionChainer<WriteParams, IDataStoreReader::ptr> chain(when_mirror_write_finishes);
		if (TurboApi::options.active_sync)
			chain.add( digestAddFunct(keyTabulator) );

		if (TurboApi::options.write_chaining && TurboApi::options.partition_keys)
			chain.add( writeChainFunct_partitionMode(ring, membership, peers) );

		chain.add( notifyWriteComplete(membership, messenger) );
		chain.add( membershipAddFunct(ring, membership, keyTabulator) );

		when_mirror_write_finishes = chain.generate();
	}

	// on drop
	{
		FunctionChainer<DropParams> chain(when_drop_finishes);
		if (TurboApi::options.active_sync)
			chain.add( digestDelFunct(keyTabulator) );
		when_drop_finishes = chain.generate();
	}
}
