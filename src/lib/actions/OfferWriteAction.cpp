/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "OfferWriteAction.h"

#include "actions_req/IMessageSender.h"
#include "common/DataBuffer.h"
#include "data_store/IDataStore.h"
#include "data_store/IDataStoreReader.h"

using std::map;
using std::string;
using std::vector;

OfferWriteAction::OfferWriteAction(const Peer& peer, const IDataStore& store, IMessageSender& messenger)
	: _peer(peer)
	, _store(store)
	, _messenger(messenger)
{
}

std::string OfferWriteAction::name() const
{
	return "offer-write";
}

bool OfferWriteAction::run(const DataBuffer& data)
{
	// check data store.
	// if name,version pair does not exist, send demand-write
	// else, do nothing and exit
	IDataStoreReader::ptr reader = _store.read(_filename, _version);
	if (!reader)
	{
		_messenger.demandWrite(_peer, _filename, _version, _source);
		return true;
	}
	return false;
}

void OfferWriteAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("name");
	if (it != params.end())
		_filename = it->second;

	it = params.find("v");
	if (it != params.end())
		_version = it->second;

	it = params.find("source");
	if (it != params.end())
		_source = it->second;
}
