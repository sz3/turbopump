/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "OfferWriteCommand.h"

#include "peer_client/IMessageSender.h"
#include "storage/IStore.h"
#include "storage/readstream.h"

#include "serialize/format.h"
#include <iostream>

OfferWriteCommand::OfferWriteCommand(const IStore& store, IMessageSender& messenger)
	: _store(store)
	, _messenger(messenger)
{
}

bool OfferWriteCommand::run(const char*, unsigned)
{
	std::cout << fmt::format("logger OfferWriteCommand {} {}", params.name, params.version) << std::endl;

	// we only respond to a write offer if we need it.
	unsigned long long offset = 0;
	readstream reader = _store.read(params.name, params.version, true);
	if (!!reader)
		offset = reader.size();

	if (offset >= params.size)
		return false;

	_messenger.demandWrite(*_peer, params.name, params.version, offset, params.source);
	return true;
}

Turbopump::Request* OfferWriteCommand::request()
{
	return &params;
}
