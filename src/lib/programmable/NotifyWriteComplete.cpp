/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "NotifyWriteComplete.h"

#include "actions_req/IMessageSender.h"
#include "common/KeyMetadata.h"
#include "membership/IMembership.h"
using std::shared_ptr;

NotifyWriteComplete::NotifyWriteComplete(const IMembership& membership, IMessageSender& messenger)
	: _membership(membership)
	, _messenger(messenger)
{
}

void NotifyWriteComplete::run(KeyMetadata md, IDataStoreReader::ptr contents)
{
	if (md.mirror >= md.totalCopies && !md.source.empty())
	{
		shared_ptr<Peer> extraMirror = _membership.lookup(md.source);
		if (extraMirror)
			_messenger.dropKey(*extraMirror, md.filename);
	}
}

