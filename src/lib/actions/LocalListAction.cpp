/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "LocalListAction.h"

#include "data_store/IDataStore.h"

LocalListAction::LocalListAction(const IDataStore& dataStore, IByteStream& writer)
	: MessageAction(dataStore.toString(), writer)
{
}
