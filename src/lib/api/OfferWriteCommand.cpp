/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "OfferWriteCommand.h"

#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"
#include "peer_client/IMessageSender.h"

OfferWriteCommand::OfferWriteCommand(const IDataStore& store, IMessageSender& messenger)
	: _store(store)
	, _messenger(messenger)
{
}

bool OfferWriteCommand::run(const char*, unsigned)
{
	// we only respond to a write offer if we need it.
	IDataStoreReader::ptr reader = _store.read(params.name, params.version);
	if (!reader)
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
