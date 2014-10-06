/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Options.h"

class IConsistentHashRing;
class ILocateKeys;
class IMembership;
class IKeyTabulator;
class IMessageSender;
class ISuperviseWrites;

// not really sure about the name of this guy, yet
class Callbacks : public Turbopump::Options
{
public:
	Callbacks(const Turbopump::Options& opts);

	void initialize(IConsistentHashRing& ring, ILocateKeys& locator, IMembership& membership, IKeyTabulator& keyTabulator, IMessageSender& messenger, ISuperviseWrites& writer);

protected:
};

