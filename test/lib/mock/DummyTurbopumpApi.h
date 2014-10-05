#pragma once

#include "api/Api.h"
#include "api/Options.h"

#include "MockDataStore.h"
#include "MockLocateKeys.h"
#include "MockMessageSender.h"
#include "MockSkewCorrector.h"
#include "MockStatusReporter.h"
#include "MockSynchronize.h"
#include "socket/StringByteStream.h"

class DummyTurbopumpApi : public Turbopump::Api
{
public:
	DummyTurbopumpApi()
		: Turbopump::Api(corrector, dataStore, locator, messenger, reporter, sync, writer, options)
	{}

public:
	Turbopump::Options options;
	MockSkewCorrector corrector;
	MockDataStore dataStore;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockStatusReporter reporter;
	MockSynchronize sync;
	StringByteStream writer;
};
