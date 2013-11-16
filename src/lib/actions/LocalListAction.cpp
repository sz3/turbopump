#include "LocalListAction.h"

#include "data_store/LocalDataStore.h"

LocalListAction::LocalListAction(const LocalDataStore& dataStore, IByteStream& writer)
	: MessageAction(dataStore.toString(), writer)
{
}
