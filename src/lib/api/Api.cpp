#include "Api.h"

#include "ListKeysOp.h"
#include "common/DataBuffer.h"

// should have a map of commands to do string -> op lookup.
// return copy of the op (refs, base type + params)

Api::Api(const IDataStore& dataStore, IByteStream& writer)
{
	_ops[Turbopump::ListKeys::name] = [&](){ return new ListKeysOp(dataStore, writer); };
}

std::unique_ptr<Op> Api::op(const std::string& command) const
{
	auto it = _ops.find(command);
	if (it == _ops.end())
		return NULL;
	return std::unique_ptr<Op>(it->second());
}

std::unique_ptr<Op> Api::op(const std::string& command, const DataBuffer& buffer) const
{
	std::unique_ptr<Op> operation = op(command);
	if (!!operation)
	{
		msgpack::unpacked msg;
		msgpack::unpack(&msg, buffer.buffer(), buffer.size());
		msg.get().convert(operation->request());
	}
	return operation;
}

std::unique_ptr<Op> Api::op(const std::string& command, const std::unordered_map<std::string,std::string>& params) const
{
	std::unique_ptr<Op> operation = op(command);
	if (!!operation)
		operation->request()->load(params);
	return operation;
}

