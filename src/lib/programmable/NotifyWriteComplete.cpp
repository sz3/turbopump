/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "NotifyWriteComplete.h"

#include "actions/WriteParams.h"
#include "actions_req/IMessageSender.h"
#include "membership/IMembership.h"
using std::shared_ptr;

NotifyWriteComplete::NotifyWriteComplete(const IMembership& membership, IMessageSender& messenger)
	: _membership(membership)
	, _messenger(messenger)
{
}

void NotifyWriteComplete::run(WriteParams& params, IDataStoreReader::ptr contents)
{
	if (params.mirror >= params.totalCopies && !params.source.empty())
	{
		shared_ptr<Peer> source = _membership.lookup(params.source);
		if (source)
			_messenger.acknowledgeWrite(*source, params.filename, params.version, contents->size());
	}
}

