/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "IMerkleIndex.h"
#include "IMerkleTree.h"
#include "SkewCorrector.h"
#include "actions_req/IWriteActionSender.h"
#include "common/KeyMetadata.h"
#include "data_store/IDataStore.h"
#include "membership/Peer.h"
#include "serialize/StringUtil.h"
#include <deque>
#include <iostream>
using std::string;

SkewCorrector::SkewCorrector(const IMerkleIndex& index, const IDataStore& store, IWriteActionSender& sender)
	: _index(index)
	, _store(store)
	, _sender(sender)
{
}

void SkewCorrector::healKey(const Peer& peer, unsigned long long key)
{
	std::cout << "how does SkewCorrector heal key? :(" << std::endl;
}

void SkewCorrector::pushKeyRange(const Peer& peer, const string& treeid, unsigned long long first, unsigned long long last)
{
	const IMerkleTree& tree = _index.find(treeid);

	// need to find all files in the key ranges, and write them to peer.
	std::deque<string> files = tree.enumerate(first, last);
	std::cout << "pushing " << files.size() << " keys to peer " << peer.uid << ": " << StringUtil::stlJoin(files) << std::endl;
	for (std::deque<string>::const_iterator it = files.begin(); it != files.end(); ++it)
	{
		//std::cout << "attempting to push file " << *it << " to peer " << peer.uid << std::endl;
		IDataStoreReader::ptr reader = _store.read(*it);
		if (!reader)
			continue;
		if (!_sender.store(peer, KeyMetadata(*it,0,0), reader))
		{
			std::cout << "uh oh, pushKeyRange is having trouble" << std::endl;
			return; // TODO: last error?
		}
	}
}
