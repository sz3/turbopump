/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "LocalStateAction.h"

#include "data_store/IDataStore.h"
#include "main/IProcessState.h"

LocalStateAction::LocalStateAction(const IProcessState& state, IByteStream& writer)
	: MessageAction(state.summary(), writer)
{
}
