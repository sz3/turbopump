/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "NotifyWriteComplete.h"

#include "api/WriteInstructions.h"
#include "membership/IKnowPeers.h"
#include "peer_client/IMessageSender.h"
#include "storage/readstream.h"
using std::shared_ptr;

NotifyWriteComplete::NotifyWriteComplete(const IKnowPeers& membership, IMessageSender& messenger)
	: _membership(membership)
	, _messenger(messenger)
{
}

void NotifyWriteComplete::run(WriteInstructions& params, readstream& contents)
{
	if (params.mirror >= params.copies && !params.source.empty())
	{
		shared_ptr<Peer> source = _membership.lookup(params.source);
		if (source)
			_messenger.acknowledgeWrite(*source, params.name, params.version, contents.size());
	}
}

