/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

// will probably make Api the interface, and name the impl something else.
// but for now just make it work.
#include "Command.h"
#include <memory>
#include <unordered_map>

class DataBuffer;
class IByteStream;
class IConsistentHashRing;
class ICorrectSkew;
class IDataStore;
class ILocateKeys;
class IKeyTabulator;
class IMembership;
class IMessageSender;
class IProcessState;
class ISynchronize;

namespace Turbopump {
class Options;

class Api
{
public:
	Api(ICorrectSkew& corrector, IDataStore& dataStore, const ILocateKeys& locator, IMessageSender& messenger, ISynchronize& sync, IByteStream& writer, const Options& options);

	std::unique_ptr<Command> command(int id, const DataBuffer& buffer) const;
	std::unique_ptr<Command> command(const std::string& name, const std::unordered_map<std::string,std::string>& params) const;

	template <typename Req>
	std::unique_ptr<Command> command(const Req& req) const
	{
		std::unique_ptr<Command> cmd(command_impl(req._ID));
		if (!cmd)
			return false;
		*(Req*)cmd->request() = req;
		return cmd;
	}

protected:
	Command* command_impl(int id) const;
	std::unique_ptr<Command> command_impl(const std::string& name) const;

protected:
	std::unordered_map<std::string, int> _commands;

	ICorrectSkew& _corrector;
	IDataStore& _dataStore;
	const ILocateKeys& _locator;
	IMessageSender& _messenger;
	ISynchronize& _sync;
	IByteStream& _writer;
	const Options& _options;
};
}//namespace
