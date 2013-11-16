#pragma once

#include "data_store/LocalDataStore.h"

class TestableDataStore : public LocalDataStore
{
public:
	using LocalDataStore::_store;
};

