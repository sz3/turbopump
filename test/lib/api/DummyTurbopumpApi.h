#pragma once

#include "Api.h"

#include "Options.h"
#include "mock/MockDataStore.h"
#include "mock/MockLocateKeys.h"
#include "socket/StringByteStream.h"

class DummyTurbopumpApi : public Turbopump::Api
{
public:
	DummyTurbopumpApi()
		: Turbopump::Api(dataStore, locator, writer, options)
	{}

public:
	Turbopump::Options options;
	MockDataStore dataStore;
	MockLocateKeys locator;
	StringByteStream writer;
};
