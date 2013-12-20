#include "LocalListAction.h"

#include "data_store/IDataStore.h"

LocalListAction::LocalListAction(const IDataStore& dataStore, IByteStream& writer)
	: MessageAction(dataStore.toString(), writer)
{
}
