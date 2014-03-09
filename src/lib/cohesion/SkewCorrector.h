/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ICorrectSkew.h"

class IDataStore;
class IKeyTabulator;
class ILog;
class IMessageSender;
class IWriteActionSender;

class SkewCorrector : public ICorrectSkew
{
public:
	SkewCorrector(const IKeyTabulator& index, const IDataStore& store, IMessageSender& messenger, IWriteActionSender& sender, ILog& logger);

	void healKey(const Peer& peer, const TreeId& treeid, unsigned long long key);
	void pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom="");
	bool sendKey(const Peer& peer, const std::string& name, const std::string& version, const std::string& source);

protected:
	const IKeyTabulator& _index;
	const IDataStore& _store;
	IMessageSender& _messenger;
	IWriteActionSender& _sender;
	ILog& _logger;
};
