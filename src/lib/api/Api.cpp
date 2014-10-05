/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Api.h"

#include "Options.h"
#include "AddPeerCommand.h"
#include "DeleteCommand.h"
#include "DropCommand.h"
#include "ListKeysCommand.h"
#include "ReadCommand.h"
#include "StatusCommand.h"
#include "WriteCommand.h"

#include "AckWriteCommand.h"
#include "DemandWriteCommand.h"
#include "HealKeyCommand.h"
#include "KeyRequestCommand.h"
#include "OfferWriteCommand.h"
#include "SyncCommand.h"
#include "common/DataBuffer.h"

// should have a map of commands to do string -> command lookup.
// return copy of the op (refs, base type + params)
namespace Turbopump {

Api::Api(ICorrectSkew& corrector, IDataStore& dataStore, const ILocateKeys& locator, IMessageSender& messenger, IStatusReporter& reporter, ISynchronize& sync, IByteStream& writer, const Options& options)
	: _corrector(corrector)
	, _dataStore(dataStore)
	, _locator(locator)
	, _messenger(messenger)
	, _reporter(reporter)
	, _sync(sync)
	, _writer(writer)
	, _options(options)
{
	// local commands: in this list.
	_commands[AddPeer::_NAME] = AddPeer::_ID;
	_commands[Delete::_NAME] = Delete::_ID;
	_commands[Drop::_NAME] = Drop::_ID;
	_commands[ListKeys::_NAME] = ListKeys::_ID;
	_commands[Read::_NAME] = Read::_ID;
	_commands[Status::_NAME] = Status::_ID;
	_commands["membership"] = Status::_ID2;
	_commands["ring"] = Status::_ID3;
	_commands[Write::_NAME] = Write::_ID;
}

// IByteStream == local
// Peer == remote
// need context for each command request?
Command* Api::command_impl(int id) const
{
	switch (id)
	{
		case AddPeer::_ID: return new AddPeerCommand(*this);
		case Delete::_ID: return new DeleteCommand(*this);
		case Drop::_ID: return new DropCommand(_dataStore, _locator, _options.when_drop_finishes);
		case ListKeys::_ID: return new ListKeysCommand(_dataStore, _writer);
		case Read::_ID: return new ReadCommand(_dataStore, _writer);
		case Status::_ID: return new StatusCommand(_reporter, _writer);
		case Status::_ID2: return new StatusCommand(_reporter, _writer, "membership");
		case Status::_ID3: return new StatusCommand(_reporter, _writer, "ring");
		case Write::_ID: return new WriteCommand(_dataStore, _options.when_local_write_finishes);
		case Write::_INTERNAL_ID: return new WriteCommand(_dataStore, _options.when_mirror_write_finishes);

		case AckWrite::_ID: return new AckWriteCommand(_dataStore, _locator, _options.when_drop_finishes);
		case DemandWrite::_ID: return new DemandWriteCommand(_corrector);
		case HealKey::_ID: return new HealKeyCommand(_corrector);
		case KeyRequest::_ID: return new KeyRequestCommand(_corrector);
		case OfferWrite::_ID: return new OfferWriteCommand(_dataStore, _messenger);
		case Sync::_ID: return new SyncCommand(_sync);

		default: return NULL;
	}
}

std::unique_ptr<Command> Api::command(int id, const char* buff, unsigned size) const
{
	// internal commands: id >= 100 ?
	std::unique_ptr<Command> operation(command_impl(id));
	if (!!operation)
	{
		msgpack::unpacked msg;
		msgpack::unpack(&msg, buff, size);
		msg.get().convert(operation->request());
	}
	return operation;
}

std::unique_ptr<Command> Api::command_impl(const std::string& name) const
{
	auto it = _commands.find(name);
	if (it == _commands.end())
		return NULL;
	return std::unique_ptr<Command>(command_impl(it->second));
}

std::unique_ptr<Command> Api::command(const std::string& name, const std::unordered_map<std::string,std::string>& params) const
{
	std::unique_ptr<Command> operation = command_impl(name);
	if (!!operation)
		operation->request()->load(params);
	return operation;
}

}//namespace
