/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SkewCorrector.h"

#include "IKeyTabulator.h"
#include "IDigestKeys.h"
#include "actions/WriteParams.h"
#include "actions_req/IWriteActionSender.h"
#include "cohesion/TreeId.h"
#include "data_store/DataEntry.h"
#include "data_store/IDataStore.h"
#include "logging/ILog.h"
#include "membership/Peer.h"

#include "serialize/StringUtil.h"
#include <deque>
#include <vector>
#include <iostream>
using std::string;

SkewCorrector::SkewCorrector(const IKeyTabulator& index, const IDataStore& store, IWriteActionSender& sender, ILog& logger)
	: _index(index)
	, _store(store)
	, _sender(sender)
	, _logger(logger)
{
}

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

	// instead of pushing, we initiate the exchange:
	// me: write proposal action
	// peer: write proposal response action (accept write / reject write)
	// me: write / don't

}

void SkewCorrector::pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom)
{
	const IDigestKeys& tree = _index.find(treeid.id, treeid.mirrors);

	// need to find all files in the key ranges, and write them to peer.
	std::deque<string> files = tree.enumerate(first, last);

	_logger.logDebug( "pushing " + StringUtil::str(files.size()) + " keys to peer " + peer.uid + ": " + StringUtil::join(files) );
	for (std::deque<string>::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		std::vector<IDataStoreReader::ptr> readers = _store.read(*it);
		for (auto read = readers.begin(); read != readers.end(); ++read)
		{
			// WriteParams sets mirror to totalCopies => "don't forward, and notify the source if there is one"
			unsigned totalCopies = (*read)->metadata().totalCopies;
			WriteParams write(*it, totalCopies, totalCopies, (*read)->metadata().version.toString());
			if (!offloadFrom.empty())
				write.source = offloadFrom;
			if (!_sender.store(peer, write, *read))
			{
				std::cout << "uh oh, pushKeyRange is having trouble" << std::endl;
				return; // TODO: last error?
			}
		}
	}
}

bool SkewCorrector::sendKey(const Peer& peer, const std::string& name, const std::string& version, const std::string& source)
{
	IDataStoreReader::ptr reader = _store.read(name, version);
	if (!reader)
		return false;

	unsigned totalCopies = reader->metadata().totalCopies;
	WriteParams write(name, totalCopies, totalCopies, version);
	write.source = source;

	if (!_sender.store(peer, write, reader))
	{
		std::cout << "uh oh, sendKey is having trouble" << std::endl;
		return false; // TODO: last error?
	}
	return true;
}
