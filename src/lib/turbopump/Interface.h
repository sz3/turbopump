/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

namespace Turbopump { class Api; }
class ILog;

class IStore;
class IKnowPeers;
class IConsistentHashRing;
class ILocateKeys;
class IKeyTabulator;

class ICorrectSkew;
class ISynchronize;
class IMessageSender;
class ISuperviseWrites;
class IWatches;

namespace Turbopump {
class Interface
{
public:
	Api& api;
	ILog& logger;

	IStore& store;
	IKnowPeers& membership;
	IConsistentHashRing& ring;
	ILocateKeys& keyLocator;
	IKeyTabulator& keyTabulator;

	ICorrectSkew& corrector;
	ISynchronize& synchronizer;
	IMessageSender& messenger;
	ISuperviseWrites& writer;
	IWatches& watches;
};
}//namespace
