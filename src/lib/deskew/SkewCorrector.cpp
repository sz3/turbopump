/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SkewCorrector.h"

#include "IKeyTabulator.h"
#include "IDigestKeys.h"
#include "api/Drop.h"
#include "api/Options.h"
#include "api/WriteInstructions.h"
#include "deskew/TreeId.h"
#include "logging/ILog.h"
#include "membership/Peer.h"
#include "peer_client/IMessageSender.h"
#include "peer_client/ISuperviseWrites.h"
#include "storage/IStore.h"
#include "storage/readstream.h"

#include "serialize/str.h"
#include "serialize/str_join.h"
#include <deque>
#include <iostream>
using std::string;

SkewCorrector::SkewCorrector(const IKeyTabulator& index, IStore& store, IMessageSender& messenger,
                             ISuperviseWrites& sender, ILog& logger, const Turbopump::Options& opts)
	: _index(index)
	, _messenger(messenger)
	, _store(store)
	, _sender(sender)
	, _logger(logger)
	, _opts(opts)
{
}

bool SkewCorrector::dropKey(const std::string& name)
{
	readstream reader = _store.read(name);
	if (!reader)
		return false;

	if (!_store.remove(name))
		return false;

	const std::function<void(const Turbopump::Drop&)>& onDrop = _opts.when_drop_finishes.fun();
	if (onDrop)
	{
		Turbopump::Drop req;
		req.name = name;
		req.copies = reader.mirrors();
		onDrop(req);
	}
	return true;
}

void SkewCorrector::pushKey(const Peer& peer, const TreeId& treeid, unsigned long long key)
{
	pushKeyRange(peer, treeid, key, key);
}

void SkewCorrector::pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom)
{
	// need to find all files in the key ranges, and write them to peer.
	const IDigestKeys& tree = _index.find(treeid.id, treeid.mirrors);

	// TODO: enumerate() has a default limit of 50 -- and no resume functionality. This is the most glaring flaw in the sync subsystem I currently know of.
	//  because the order is deterministic, there may be cases where we fail to sync (because we can never "reach" those files).
	//  Perhaps we should use forEachInRange() instead?
	//  alternatively, introduce some randomness or state.
	std::deque<string> files = tree.enumerate(first, last);
	if (files.empty())
	{
		std::cerr << "welp, pushKeyRange found nothing. :(" << std::endl;
		return;
	}

	_logger.logDebug( "pushing " + turbo::str::str(files.size()) + " keys to peer " + peer.uid + ": " + turbo::str::join(files) );
	for (const string& file : files)
	{
		// if file has expired, don't offer. drop instead.
		std::vector<readstream> streams = _store.readAll(file);
		if ( streams.size() == 1 && _store.isExpired(streams.front().version()) )
		{
			dropKey(file);
			continue;
		}

		for (const readstream& stream : streams)
			_messenger.offerWrite(peer, file, stream.version(), stream.size(), offloadFrom);
	}
}

bool SkewCorrector::sendKey(const Peer& peer, const std::string& name, const std::string& version, unsigned long long offset, const std::string& source)
{
	readstream reader = _store.read(name, version);
	if (!reader)
		return false;

	unsigned totalCopies = reader.mirrors();
	WriteInstructions write;
	write.name = name;
	write.copies = totalCopies;
	write.mirror = totalCopies;
	write.version = version;
	write.offset = offset;
	write.source = source;
	write.isComplete = true;

	if (!_sender.store(peer, write, reader))
	{
		_logger.logError("sendKey failed to store file [" + name + "," + version + "] to peer " + peer.uid);
		return false; // TODO: last error?
	}
	return true;
}
