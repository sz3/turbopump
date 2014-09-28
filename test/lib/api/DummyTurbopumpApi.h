#pragma once

#include "Api.h"

#include "Options.h"
#include "mock/MockDataStore.h"
#include "mock/MockLocateKeys.h"
#include "mock/MockMessageSender.h"
#include "mock/MockSkewCorrector.h"
#include "mock/MockSynchronize.h"
#include "socket/StringByteStream.h"

class DummyTurbopumpApi : public Turbopump::Api
{
public:
	DummyTurbopumpApi()
		: Turbopump::Api(corrector, dataStore, locator, messenger, sync, writer, options)
	{}

public:
	Turbopump::Options options;
	MockSkewCorrector corrector;
	MockDataStore dataStore;
	MockLocateKeys locator;
	MockMessageSender messenger;
	MockSynchronize sync;
	StringByteStream writer;
};
