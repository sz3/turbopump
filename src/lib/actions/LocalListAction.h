#pragma once

#include "MessageAction.h"

class IByteStream;
class LocalDataStore;

class LocalListAction : public MessageAction
{
public:
	LocalListAction(const LocalDataStore& dataStore, IByteStream& writer);
};

