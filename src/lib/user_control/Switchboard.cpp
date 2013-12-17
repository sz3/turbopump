#include "Switchboard.h"

// actions
#include "actions/LocalListAction.h"
#include "actions/ReadAction.h"
#include "actions/ViewMembershipAction.h"
#include "actions/WriteAction.h"

#include "common/ActionParser.h"
#include "common/DataBuffer.h"
#include "membership/IMembership.h"
#include "socket/IByteStream.h"
#include "programmable/Callbacks.h"

#include <iostream>
#include <vector>
using std::string;

Switchboard::Switchboard(IByteStream& stream, IDataStore& dataStore, const LocalDataStore& localDataStore, const IMembership& membership, const Callbacks& callbacks)
	: _stream(stream)
	, _dataStore(dataStore)
	, _localDataStore(localDataStore)
	, _membership(membership)
	, _callbacks(callbacks)
{
}

void Switchboard::run()
{
	std::unique_ptr<IAction> action;
	std::vector<char> buff;
	buff.resize(8192);

	// some sort of length based packet format: length|action|params|[data][newline]
	// if first token starts with a char, it's an action, no length. (run only one action)
	//
	//  * stream.read
	//  * while (parse(buff))
	//  *    do stuff
	//  * save remainder + size into buff, append next read

	while (1)
	{
		int bytesRead = _stream.read(&buff[0], buff.capacity());
		if (bytesRead <= 0)
			break;

		DataBuffer data(buff.data(), bytesRead);
		if (!parse(data, action) && !action)
		{
			string dumbuff(buff.data(), bytesRead);
			std::cout << "failed to parse action out of packet size " << bytesRead << ":" << dumbuff << std::endl;
			break;
		}
		action->run(data);
	}
}

bool Switchboard::parse(DataBuffer& data, std::unique_ptr<IAction>& action)
{
	ActionParser parser;
	if (!parser.parse(data))
		return false;

	action = newAction(parser.action(), parser.params());
	return action && action->good();
}

std::unique_ptr<IAction> Switchboard::newAction(const string& actionName, const std::map<string,string>& params)
{
	std::unique_ptr<IAction> action;
	if (actionName == "write")
		action.reset(new WriteAction(_dataStore, _callbacks.when_local_write_finishes));
	else if (actionName == "read")
		action.reset(new ReadAction(_dataStore, _stream));
	else if (actionName == "local_list")
		action.reset(new LocalListAction(_localDataStore, _stream));
	else if (actionName == "membership")
		action.reset(new ViewMembershipAction(_membership, _stream));
	else
		return action;

	action->setParams(params);
	return action;
}
