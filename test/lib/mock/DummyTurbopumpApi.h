/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Api.h"
#include "api/Options.h"

#include "MockLocateKeys.h"
#include "MockMessageSender.h"
#include "MockSkewCorrector.h"
#include "MockStatusReporter.h"
#include "MockStore.h"
#include "MockSynchronize.h"

class DummyTurbopumpApi : public Turbopump::Api
{
public:
	DummyTurbopumpApi()
		: Turbopump::Api(corrector, locator, messenger, reporter, store, sync, options)
	{}

public:
	Turbopump::Options options;
	MockSkewCorrector corrector;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockStore store;
	MockSynchronize sync;
};
