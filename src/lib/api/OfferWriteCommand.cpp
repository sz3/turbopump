/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "OfferWriteCommand.h"

#include "peer_client/IMessageSender.h"
#include "storage/IStore.h"

OfferWriteCommand::OfferWriteCommand(const IStore& store, IMessageSender& messenger)
	: _store(store)
	, _messenger(messenger)
{
}

bool OfferWriteCommand::run(const char*, unsigned)
{
	// we only respond to a write offer if we need it.
	if ( !_store.exists(params.name, params.version) )
	{
		_messenger.demandWrite(*_peer, params.name, params.version, params.source);
		return true;
	}
	return false;
}

Turbopump::Request* OfferWriteCommand::request()
{
	return &params;
}
