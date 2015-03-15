/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "BuildCallbacks.h"

#include "AddPeer.h"
#include "ChainWrite.h"
#include "MirrorToPeer.h"
#include "NotifyWriteComplete.h"
#include "RandomizedMirrorToPeer.h"

#include "api/Drop.h"
#include "deskew/IKeyTabulator.h"
#include "storage/readstream.h"
#include "turbopump/Turbopump.h"

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

BuildCallbacks::BuildCallbacks(Turbopump::Options& opts)
	: _opts(opts)
{
}

void BuildCallbacks::build(Turbopump::Turbopump& turbopump, IKeyTabulator& keyTabulator, IMessageSender& messenger, ISuperviseWrites& writer)
{
	// on local write
	{
		if (_opts.active_sync)
			_opts.when_local_write_finishes.add( digestAddFunct(keyTabulator) );

		if (_opts.write_chaining)
		{
			if (_opts.partition_keys)
				_opts.when_local_write_finishes.add( writeChainFunct_partitionMode(turbopump.keyLocator, turbopump.membership, writer, true) );
			else
				_opts.when_local_write_finishes.add( writeChainFunct_cloneMode(turbopump.keyLocator, turbopump.membership, writer, true) );
		}
		_opts.when_local_write_finishes.add( membershipAddFunct(turbopump.ring, turbopump.membership, keyTabulator) );
	}

	// on mirror write
	{
		if (_opts.active_sync)
			_opts.when_mirror_write_finishes.add( digestAddFunct(keyTabulator) );

		if (_opts.write_chaining && _opts.partition_keys)
			_opts.when_mirror_write_finishes.add( writeChainFunct_partitionMode(turbopump.keyLocator, turbopump.membership, writer, false) );

		_opts.when_mirror_write_finishes.add( notifyWriteComplete(turbopump.membership, messenger) );
		_opts.when_mirror_write_finishes.add( membershipAddFunct(turbopump.ring,turbopump. membership, keyTabulator) );
	}

	// on drop
	{
		if (_opts.active_sync)
			_opts.when_drop_finishes.add( digestDelFunct(keyTabulator) );
	}

	if (_opts.build_callbacks)
		_opts.build_callbacks(_opts, turbopump);

	_opts.when_local_write_finishes.finalize();
	_opts.when_mirror_write_finishes.finalize();
	_opts.when_drop_finishes.finalize();
}
