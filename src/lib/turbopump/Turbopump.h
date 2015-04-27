#pragma once

#include "api/Api.h"
#include "api/Options.h"
#include "common/KeyMetadata.h"
#include "common/ProcessState.h"
#include "common/StatusReporter.h"
#include "deskew/KeyTabulator.h"
#include "deskew/SkewCorrector.h"
#include "deskew/Synchronizer.h"
#include "hashing/ConsistentHashRing.h"
#include "hashing/LocateKeys.h"
#include "logging/StderrLogger.h"
#include "membership/Membership.h"
#include "membership/Peer.h"
#include "storage/FileStore.h"

#include "socket/socket_address.h"
#include "serialize/str.h"

class IMessageSender;
class ISuperviseWrites;

namespace Turbopump {
class Turbopump
{
public:
	Turbopump(const Options& opts, IMessageSender& messenger, ISuperviseWrites& sender)
		: api(corrector, keyLocator, messenger, reporter, store, synchronizer, opts)
		, logger(socket_address("127.0.0.1", opts.internal_port).toString())
		, reporter(ring, membership, state)
		, keyLocator(ring, membership)
		, keyTabulator(keyLocator)
		, store(turbo::str::str(opts.internal_port) + "/store")
		, membership("turbo_members.txt", socket_address("127.0.0.1", opts.internal_port).toString())
		, corrector(keyTabulator, store, messenger, sender, logger)
		, synchronizer(ring, membership, keyTabulator, messenger, corrector, logger)
	{
	}

	void preStart(const Options& opts)
	{
		// membership
		if (membership.load())
			membership.syncToDataStore(store);
		else
		{
			logger.logWarn("failed to load membership.");
			membership.addSelf();
		}

		if (opts.partition_keys)
		{
			ConsistentHashRing& chr(ring);
			auto fun = [&chr] (const Peer& peer) { chr.insert(peer.uid, peer.uid); };
			membership.forEachPeer(fun);
		}
	}

	void postStart(const Options& opts)
	{
		if (opts.active_sync)
		{
			KeyTabulator& tabulator(keyTabulator);
			auto fun = [&tabulator] (const std::string& name, const KeyMetadata& md, const std::string&)
			{
				tabulator.update(name, md.digest, md.totalCopies);
				return true;
			};
			store.enumerate(fun, ~0ULL);
		}
	}

public:
	// api
	Api api;

	// misc
	StderrLogger logger;
	ProcessState state;
	StatusReporter reporter;

	// partitioning
	ConsistentHashRing ring;
	LocateKeys keyLocator;
	KeyTabulator keyTabulator;

	// storage, membership
	FileStore store;
	Membership membership;

	// sync, dependent on internal messaging
	SkewCorrector corrector;
	Synchronizer synchronizer;
};
}//namespace
