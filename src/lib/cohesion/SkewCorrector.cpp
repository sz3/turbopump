/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "SkewCorrector.h"

#include "IKeyTabulator.h"
#include "IDigestKeys.h"
#include "actions/WriteParams.h"
#include "actions_req/IWriteActionSender.h"
#include "cohesion/TreeId.h"
#include "data_store/DataEntry.h"
#include "data_store/IDataStore.h"
#include "membership/Peer.h"

#include "serialize/StringUtil.h"
#include <deque>
#include <iostream>
using std::string;

SkewCorrector::SkewCorrector(const IKeyTabulator& index, const IDataStore& store, IWriteActionSender& sender)
	: _index(index)
	, _store(store)
	, _sender(sender)
{
}

void SkewCorrector::healKey(const Peer& peer, unsigned long long key)
{
	std::cout << "how does SkewCorrector heal key? :(" << std::endl;
}

void SkewCorrector::pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom)
{
	const IDigestKeys& tree = _index.find(treeid.id, treeid.mirrors);

	// need to find all files in the key ranges, and write them to peer.
	std::deque<string> files = tree.enumerate(first, last);
	std::cout << "pushing " << files.size() << " keys to peer " << peer.uid << ": " << StringUtil::join(files) << std::endl;
	for (std::deque<string>::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		IDataStoreReader::ptr reader = _store.read(*it);
		if (!reader)
			continue;

		WriteParams write(*it, 0, reader->data().totalCopies);
		if (!offloadFrom.empty())
		{
			write.source = offloadFrom;
			write.mirror = write.totalCopies;
		}
		if (!_sender.store(peer, write, reader))
		{
			std::cout << "uh oh, pushKeyRange is having trouble" << std::endl;
			return; // TODO: last error?
		}
	}
}
