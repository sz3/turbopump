/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "TurboApi.h"

class IHashRing;
class ILocateKeys;
class IMembership;
class IKeyTabulator;
class IMessageSender;
class ISuperviseWrites;

// not really sure about the name of this guy, yet
class Callbacks : public TurboApi
{
public:
	Callbacks();
	Callbacks(const TurboApi& instruct);

	void initialize(IHashRing& ring, ILocateKeys& locator, IMembership& membership, IKeyTabulator& keyTabulator, IMessageSender& messenger, ISuperviseWrites& writer);

protected:
};

