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
	const IDigestKeys& tree = _index.find(treeid.id, treeid.mirrors);

	// need to find all files in the key ranges, and write them to peer.
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
		std::vector<string> versions = _store.versions(file, true);
		if ( versions.size() == 1 && _store.isExpired(versions.front()) )
		{
			dropKey(file);
			continue;
		}

		for (const string& version : versions)
			_messenger.offerWrite(peer, file, version, offloadFrom);
	}
}

bool SkewCorrector::sendKey(const Peer& peer, const std::string& name, const std::string& version, const std::string& source)
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
	write.source = source;
	write.isComplete = true;

	if (!_sender.store(peer, write, reader))
	{
		_logger.logError("sendKey failed to store file [" + name + "," + version + "] to peer " + peer.uid);
		return false; // TODO: last error?
	}
	return true;
}
