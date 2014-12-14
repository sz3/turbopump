/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ICorrectSkew.h"

class IKeyTabulator;
class ILog;
class IMessageSender;
class IStore;
class ISuperviseWrites;

class SkewCorrector : public ICorrectSkew
{
public:
	SkewCorrector(const IKeyTabulator& index, const IStore& store, IMessageSender& messenger, ISuperviseWrites& sender, ILog& logger);

	void healKey(const Peer& peer, const TreeId& treeid, unsigned long long key);
	void pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom="");
	bool sendKey(const Peer& peer, const std::string& name, const std::string& version, const std::string& source);

protected:
	const IKeyTabulator& _index;
	const IStore& _store;
	IMessageSender& _messenger;
	ISuperviseWrites& _sender;
	ILog& _logger;
};
