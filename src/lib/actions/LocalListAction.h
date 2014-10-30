/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "MessageAction.h"

class IByteStream;
class IDataStore;

class LocalListAction : public MessageAction
{
public:
	LocalListAction(const IDataStore& dataStore, IByteStream& writer);
};

