/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ComputedOptions.h"

#include "ChainWrite.h"
#include "MirrorToPeer.h"
#include "ModifyPeer.h"
#include "NotifyWriteComplete.h"
#include "RandomizedMirrorToPeer.h"

#include "api/Drop.h"
#include "callbacks/IWatches.h"
#include "deskew/IKeyTabulator.h"
#include "storage/readstream.h"
#include "turbopump/Interface.h"

#include <deque>
#include <functional>
using std::bind;
using std::deque;
using std::function;
using namespace std::placeholders;

// TODO: rather than anonymous namespace, should split these functions out somewhere else...
namespace
{
	std::function<void(WriteInstructions&, readstream&)> membershipFunct(IConsistentHashRing& ring, IKnowPeers& membership, IKeyTabulator& keyTabulator)
	{
		return [&] (WriteInstructions& params, readstream& contents)
		{
			ModifyPeer peerer(ring, membership, keyTabulator);
			peerer.run(params, contents);
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

	std::function<void(WriteInstructions&, readstream&)> notifyWriteComplete(const IKnowPeers& membership, IMessageSender& messenger)
	{
		std::shared_ptr<NotifyWriteComplete> cmd(new NotifyWriteComplete(membership, messenger));
		return [cmd] (WriteInstructions& params, readstream& contents)
		{
			cmd->run(params, contents);
		};
	}

	std::function<void(WriteInstructions&, readstream&)> writeChainFunct_cloneMode(const ILocateKeys& locator, const IKnowPeers& membership, ISuperviseWrites& writer, bool blocking)
	{
		std::shared_ptr< ChainWrite<RandomizedMirrorToPeer> > cmd(new ChainWrite<RandomizedMirrorToPeer>(locator, membership, writer, blocking));
		return [cmd] (WriteInstructions& params, readstream& contents)
		{
			cmd->run(params, contents);
		};
	}

	std::function<void(WriteInstructions&, readstream&)> writeChainFunct_partitionMode(const ILocateKeys& locator, const IKnowPeers& membership, ISuperviseWrites& writer, bool blocking)
	{
		std::shared_ptr< ChainWrite<MirrorToPeer> > cmd(new ChainWrite<MirrorToPeer>(locator, membership, writer, blocking));
		return [cmd] (WriteInstructions& params, readstream& contents)
		{
			cmd->run(params, contents);
		};
	}

	std::function<void(WriteInstructions&, readstream&)> notifyWatches(const IWatches& watches)
	{
		return [&] (WriteInstructions& params, readstream&)
		{
			watches.notify(params.name);
		};
	}
}

ComputedOptions::ComputedOptions(const Turbopump::Options& opts, const Turbopump::Interface& turbopump)
	: Turbopump::Options(opts)
{
	// on local write
	{
		if (active_sync)
			when_local_write_finishes.add( digestAddFunct(turbopump.keyTabulator) );

		if (write_chaining)
		{
			if (partition_keys)
				when_local_write_finishes.add( writeChainFunct_partitionMode(turbopump.keyLocator, turbopump.membership, turbopump.writer, true) );
			else
				when_local_write_finishes.add( writeChainFunct_cloneMode(turbopump.keyLocator, turbopump.membership, turbopump.writer, true) );
		}
		when_local_write_finishes.add( notifyWatches(turbopump.watches) );
		when_local_write_finishes.add( membershipFunct(turbopump.ring, turbopump.membership, turbopump.keyTabulator) );
	}

	// on mirror write
	{
		if (active_sync)
			when_mirror_write_finishes.add( digestAddFunct(turbopump.keyTabulator) );

		if (write_chaining && partition_keys)
			when_mirror_write_finishes.add( writeChainFunct_partitionMode(turbopump.keyLocator, turbopump.membership, turbopump.writer, false) );

		when_mirror_write_finishes.add( notifyWriteComplete(turbopump.membership, turbopump.messenger) );
		when_mirror_write_finishes.add( notifyWatches(turbopump.watches) );
		when_mirror_write_finishes.add( membershipFunct(turbopump.ring,turbopump. membership, turbopump.keyTabulator) );
	}

	// on drop
	{
		if (active_sync)
			when_drop_finishes.add( digestDelFunct(turbopump.keyTabulator) );
	}

	if (build_callbacks)
		build_callbacks(*this, turbopump);

	when_local_write_finishes.finalize();
	when_mirror_write_finishes.finalize();
	when_drop_finishes.finalize();
}
