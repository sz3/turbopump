/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Api.h"

#include "Options.h"
#include "DropCommand.h"
#include "ListKeysCommand.h"
#include "ReadCommand.h"
#include "WriteCommand.h"
#include "common/DataBuffer.h"

// should have a map of commands to do string -> command lookup.
// return copy of the op (refs, base type + params)
namespace Turbopump {

Api::Api(IDataStore& dataStore, const ILocateKeys& locator, IByteStream& writer, const Options& options)
	: _dataStore(dataStore)
	, _locator(locator)
	, _writer(writer)
	, _options(options)
{
	// local commands: in this list.
	_commands[Drop::NAME] = Drop::ID;
	_commands[Read::NAME] = Read::ID;
	_commands[ListKeys::NAME] = ListKeys::ID;
	_commands[Write::NAME] = Write::LOCAL_ID;
}

Command* Api::command(int id) const
{
	switch (id)
	{
		case Drop::ID: return new DropCommand(_dataStore, _locator, _options.when_drop_finishes);
		case ListKeys::ID: return new ListKeysCommand(_dataStore, _writer);
		case Read::ID: return new ReadCommand(_dataStore, _writer);
		case Write::LOCAL_ID: return new WriteCommand(_dataStore, _options.when_local_write_finishes);
		case Write::ID: return new WriteCommand(_dataStore, _options.when_mirror_write_finishes);

		default: return NULL;
	}
}

std::unique_ptr<Command> Api::command(const std::string& name) const
{
	auto it = _commands.find(name);
	if (it == _commands.end())
		return NULL;
	return std::unique_ptr<Command>(command(it->second));
}

std::unique_ptr<Command> Api::command(int id, const DataBuffer& buffer) const
{
	// internal commands: id >= 100 ?
	std::unique_ptr<Command> operation(command(id));
	if (!!operation)
	{
		msgpack::unpacked msg;
		msgpack::unpack(&msg, buffer.buffer(), buffer.size());
		msg.get().convert(operation->request());
	}
	return operation;
}

std::unique_ptr<Command> Api::command(const std::string& name, const std::unordered_map<std::string,std::string>& params) const
{
	std::unique_ptr<Command> operation = command(name);
	if (!!operation)
		operation->request()->load(params);
	return operation;
}

}//namespace
