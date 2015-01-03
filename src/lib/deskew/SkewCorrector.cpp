/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SkewCorrector.h"

#include "IKeyTabulator.h"
#include "IDigestKeys.h"
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
#include <vector>
#include <iostream>
using std::string;

SkewCorrector::SkewCorrector(const IKeyTabulator& index, const IStore& store, IMessageSender& messenger, ISuperviseWrites& sender, ILog& logger)
	: _index(index)
	, _messenger(messenger)
	, _store(store)
	, _sender(sender)
	, _logger(logger)
{
}

// test this!
void SkewCorrector::healKey(const Peer& peer, const TreeId& treeid, unsigned long long key)
{
	const IDigestKeys& tree = _index.find(treeid.id, treeid.mirrors);

	// need to find all files in the key ranges, and write them to peer.
	std::deque<string> files = tree.enumerate(key, key);
	if (files.empty())
	{
		std::cerr << "welp, healKey found nothing. :(" << std::endl;
		return;
	}

	// we initiate the exchange:
	// me: propose write action
	// peer: response -> demand write action
	// me: write / don't
	for (std::deque<string>::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		std::vector<string> versions = _store.versions(*it);
		for (const string& version : versions)
			_messenger.offerWrite(peer, *it, version, "");
	}
}

void SkewCorrector::pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom)
{
	const IDigestKeys& tree = _index.find(treeid.id, treeid.mirrors);

	// need to find all files in the key ranges, and write them to peer.
	std::deque<string> files = tree.enumerate(first, last);

	_logger.logDebug( "pushing " + turbo::str::str(files.size()) + " keys to peer " + peer.uid + ": " + turbo::str::join(files) );
	for (const string& file : files)
	{
		std::vector<readstream> readers = _store.readAll(file);
		for (readstream& read : readers)
		{
			// WriteInstructions sets mirror to totalCopies => "don't forward, and notify the source if there is one"
			unsigned totalCopies = read.mirrors();
			WriteInstructions write;
			write.name = file;
			write.copies = totalCopies;
			write.mirror = totalCopies;
			write.version = read.version();
			if (!offloadFrom.empty())
				write.source = offloadFrom;
			write.isComplete = true;
			if (!_sender.store(peer, write, read))
			{
				std::cout << "uh oh, pushKeyRange is having trouble" << std::endl;
				return; // TODO: last error?
			}
		}
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
