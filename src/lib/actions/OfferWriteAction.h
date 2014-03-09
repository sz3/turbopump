/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"
#include "membership/Peer.h"
class IDataStore;
class IMessageSender;

class OfferWriteAction : public IAction
{
public:
	OfferWriteAction(const Peer& peer, const IDataStore& store, IMessageSender& messenger);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	Peer _peer;
	const IDataStore& _store;
	IMessageSender& _messenger;

	std::string _filename;
	std::string _version;
	std::string _source;
};
