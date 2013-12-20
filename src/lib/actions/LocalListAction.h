#pragma once

#include "MessageAction.h"

class IByteStream;
class IDataStore;

class LocalListAction : public MessageAction
{
public:
	LocalListAction(const IDataStore& dataStore, IByteStream& writer);
};

