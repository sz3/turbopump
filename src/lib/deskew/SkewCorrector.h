/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "ICorrectSkew.h"
#include <vector>

namespace Turbopump { class Options; }

class IKeyTabulator;
class ILog;
class IMessageSender;
class IStore;
class ISuperviseWrites;

class SkewCorrector : public ICorrectSkew
{
public:
	SkewCorrector(const IKeyTabulator& index, IStore& store, IMessageSender& messenger, ISuperviseWrites& sender, ILog& logger, const Turbopump::Options& opts);

	bool dropKey(const std::string& name);

	void pushKey(const Peer& peer, const TreeId& treeid, unsigned long long key);
	void pushKeyRange(const Peer& peer, const TreeId& treeid, unsigned long long first, unsigned long long last, const std::string& offloadFrom="");
	bool sendKey(const Peer& peer, const std::string& name, const std::string& version, unsigned long long offset, const std::string& source);

protected:
	const IKeyTabulator& _index;
	IStore& _store;
	IMessageSender& _messenger;
	ISuperviseWrites& _sender;
	ILog& _logger;
	const Turbopump::Options& _opts;
};
