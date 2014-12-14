/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Callbacks.h"

#include "AddPeer.h"
#include "ChainWrite.h"
#include "MirrorToPeer.h"
#include "NotifyWriteComplete.h"
#include "RandomizedMirrorToPeer.h"

#include "api/Drop.h"
#include "deskew/IKeyTabulator.h"
#include "storage/readstream.h"

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
	std::function<void(WriteInstructions&, readstream&)> membershipAddFunct(IConsistentHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator)
	{
		return [&] (WriteInstructions& params, readstream& contents)
		{
			AddPeer adder(ring, membership, keyTabulator);
			adder.run(params, contents);
		};
	}

	std::function<void(WriteInstructions&, readstream&)> digestAddFunct(IKeyTabulator& keyTabulator)
	{
		return [&] (WriteInstructions& params, readstream& contents)
		{
			keyTabulator.update(params.name, params.digest xor contents.digest(), params.copies);
		};
	}

	std::function<void(Turbopump::Drop)> digestDelFunct(IKeyTabulator& keyTabulator)
	{
		return [&] (Turbopump::Drop params)
		{
			keyTabulator.remove(params.name, params.copies);
		};
	}

	std::function<void(WriteInstructions&, readstream&)> notifyWriteComplete(const IMembership& membership, IMessageSender& messenger)
	{
		std::shared_ptr<NotifyWriteComplete> cmd(new NotifyWriteComplete(membership, messenger));
		return bind(&NotifyWriteComplete::run, cmd, _1, _2);
	}

	std::function<void(WriteInstructions&, readstream&)> writeChainFunct_cloneMode(const ILocateKeys& locator, const IMembership& membership, ISuperviseWrites& writer, bool blocking)
	{
		std::shared_ptr< ChainWrite<RandomizedMirrorToPeer> > cmd(new ChainWrite<RandomizedMirrorToPeer>(locator, membership, writer, blocking));
		return bind(&ChainWrite<RandomizedMirrorToPeer>::run, cmd, _1, _2);
	}

	std::function<void(WriteInstructions&, readstream&)> writeChainFunct_partitionMode(const ILocateKeys& locator, const IMembership& membership, ISuperviseWrites& writer, bool blocking)
	{
		std::shared_ptr< ChainWrite<MirrorToPeer> > cmd(new ChainWrite<MirrorToPeer>(locator, membership, writer, blocking));
		return bind(&ChainWrite<MirrorToPeer>::run, cmd, _1, _2);
	}
}

Callbacks::Callbacks(const Turbopump::Options& opts)
	: Turbopump::Options(opts)
{
}

void Callbacks::initialize(IConsistentHashRing& ring, ILocateKeys& locator, IMembership& membership, IKeyTabulator& keyTabulator, IMessageSender& messenger, ISuperviseWrites& writer)
{
	// TODO: devise a proper callback strategy for configurable default callbacks + user defined ones.
	//  yes, I know this is basically: "TODO: figure out how to land on moon"

	// on local write
	{
		FunctionChainer<WriteInstructions&, readstream&> chain(when_local_write_finishes);
		if (active_sync)
			chain.add( digestAddFunct(keyTabulator) );

		if (write_chaining)
		{
			if (partition_keys)
				chain.add( writeChainFunct_partitionMode(locator, membership, writer, true) );
			else
				chain.add( writeChainFunct_cloneMode(locator, membership, writer, true) );
		}
		chain.add( membershipAddFunct(ring, membership, keyTabulator) );

		when_local_write_finishes = chain.generate();
	}

	// on mirror write
	{
		FunctionChainer<WriteInstructions&, readstream&> chain(when_mirror_write_finishes);
		if (active_sync)
			chain.add( digestAddFunct(keyTabulator) );

		if (write_chaining && partition_keys)
			chain.add( writeChainFunct_partitionMode(locator, membership, writer, false) );

		chain.add( notifyWriteComplete(membership, messenger) );
		chain.add( membershipAddFunct(ring, membership, keyTabulator) );

		when_mirror_write_finishes = chain.generate();
	}

	// on drop
	{
		FunctionChainer<Turbopump::Drop> chain(when_drop_finishes);
		if (active_sync)
			chain.add( digestDelFunct(keyTabulator) );
		when_drop_finishes = chain.generate();
	}
}
