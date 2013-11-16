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
	std::vector<char> buff;
	buff.resize(8192);
	while (1)
	{
		int bytesRead = _stream.read(&buff[0], buff.capacity());
		if (bytesRead <= 0)
			break;
		parse(buff.data(), bytesRead);
	}
}

// TODO: this won't fly. We need something like the IAsyncReader to wrap the ByteStream,
// to enable data larger than a single packet. The ActionParser will also need to be involved,
// since the best way to treat the data will be to pull the action off the front of the buffer,
// and pass the rest on.
// potentially: _stream.getCommand(command)?
void Switchboard::parse(const char* buffer, unsigned size)
{
	{
		string dumbuff(buffer, buffer+size);
		std::cout << size << ":" << dumbuff << std::endl;
	}

	ActionParser parser;
	DataBuffer data(buffer, size);
	if (!parser.parse(data))
		return;

	std::shared_ptr<IAction> action = newAction(parser.action(), parser.params());
	if (action && action->good())
		action->run(data);
}

std::shared_ptr<IAction> Switchboard::newAction(const string& actionName, const std::map<string,string>& params)
{
	std::shared_ptr<IAction> action;
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
