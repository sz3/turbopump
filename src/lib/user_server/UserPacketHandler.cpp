/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "UserPacketHandler.h"

// actions
#include "actions/AddPeerAction.h"
#include "actions/DeleteAction.h"
#include "actions/LocalListAction.h"
#include "actions/LocalStateAction.h"
#include "actions/ReadAction.h"
#include "actions/ViewConsistentHashRingAction.h"
#include "actions/ViewMembershipAction.h"
#include "actions/WriteAction.h"

#include "UserActionContext.h"
#include "http/IHttpByteStream.h"
#include "membership/IMembership.h"
#include "programmable/TurboApi.h"

#include <vector>
using std::string;

UserPacketHandler::UserPacketHandler(IHttpByteStream& stream, IDataStore& dataStore, IConsistentHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator, const IProcessState& state, const TurboApi& callbacks)
	: _stream(stream)
	, _dataStore(dataStore)
	, _ring(ring)
	, _membership(membership)
	, _keyTabulator(keyTabulator)
	, _state(state)
	, _callbacks(callbacks)
{
}

void UserPacketHandler::run()
{
	UserActionContext actionContext(*this);
	std::vector<char> buff;
	buff.resize(8192);

	while (1)
	{
		int bytesRead = _stream.read(&buff[0], buff.capacity());
		if (bytesRead <= 0)
			break;

		if (!actionContext.feed(buff.data(), bytesRead))
			break;
	}
}

void UserPacketHandler::sendResponse(StatusCode status)
{
	_stream.setStatus(status.integer());
	_stream.write(NULL, 0);
}

std::unique_ptr<IAction> UserPacketHandler::newAction(const string& actionName, const std::map<string,string>& params) const
{
	// TODO: proper REST resource hanging?
	//Switchboard resources;
	//resources.put("/data/[name]", WriteAction::params());
	//resources.get("/data/[name]", ReadAction::params());

	std::unique_ptr<IAction> action;
	if (actionName == "write")
		action.reset(new WriteAction(_dataStore, _callbacks.when_local_write_finishes));
	else if (actionName == "read")
		action.reset(new ReadAction(_dataStore, _stream));
	else if (actionName == "delete")
		action.reset(new DeleteAction(std::move(newAction("write", std::map<string,string>()))));
	else if (actionName == "local_list")
		action.reset(new LocalListAction(_dataStore, _stream));
	else if (actionName == "membership")
		action.reset(new ViewMembershipAction(_membership, _stream));
	else if (actionName == "ring")
		action.reset(new ViewConsistentHashRingAction(_ring, _stream));
	else if (actionName == "state")
		action.reset(new LocalStateAction(_state, _stream));
	else if (actionName == "add_peer")
		action.reset(new AddPeerAction(std::move(newAction("write", std::map<string,string>()))));
	else
		return action;

	action->setParams(params);
	return action;
}
