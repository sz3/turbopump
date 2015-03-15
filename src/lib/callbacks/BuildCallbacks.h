/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Options.h"

namespace Turbopump { class Turbopump; }

class IKeyTabulator;
class IMessageSender;
class ISuperviseWrites;

class BuildCallbacks
{
public:
	BuildCallbacks(Turbopump::Options& opts);

	void build(Turbopump::Turbopump& turbopump, IKeyTabulator& keyTabulator, IMessageSender& messenger, ISuperviseWrites& writer);

protected:
	Turbopump::Options& _opts;
};

