#include "Api.h"

#include "ListKeysCommand.h"
#include "common/DataBuffer.h"

// should have a map of commands to do string -> command lookup.
// return copy of the op (refs, base type + params)

Api::Api(const IDataStore& dataStore, IByteStream& writer)
{
	_commands[Turbopump::ListKeys::NAME] = [&](){ return new ListKeysCommand(dataStore, writer); };
}

std::unique_ptr<Command> Api::command(const std::string& name) const
{
	auto it = _commands.find(name);
	if (it == _commands.end())
		return NULL;
	return std::unique_ptr<Command>(it->second());
}

std::unique_ptr<Command> Api::command(const std::string& name, const DataBuffer& buffer) const
{
	std::unique_ptr<Command> operation = command(name);
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

