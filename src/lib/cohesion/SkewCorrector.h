/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ICorrectSkew.h"

class IDataStore;
class IKeyTabulator;
class ILog;
class IWriteActionSender;

class SkewCorrector : public ICorrectSkew
{
public:
	SkewCorrector(const IKeyTabulator& index, const IDataStore& store, IWriteActionSender& sender, ILog& logger);

	void healKey(const Peer& peer, unsigned long long key);
	void pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom="");

protected:
	const IKeyTabulator& _index;
	const IDataStore& _store;
	IWriteActionSender& _sender;
	ILog& _logger;
};
