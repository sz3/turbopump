/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "OfferWrite.h"

#include "InternalCommand.h"
class IDataStore;
class IMessageSender;

class OfferWriteCommand : public InternalCommand
{
public:
	OfferWriteCommand(const IDataStore& store, IMessageSender& messenger);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	const IDataStore& _store;
	IMessageSender& _messenger;

public:
	Turbopump::OfferWrite params;
};
