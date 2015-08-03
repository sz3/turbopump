/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

// will probably make Api the interface, and name the impl something else.
// but for now just make it work.
#include "Command.h"
#include <memory>
#include <unordered_map>

class ICorrectSkew;
class ILocateKeys;
class IMessageSender;
class IStatusReporter;
class IStore;
class ISynchronize;

namespace Turbopump {
class Options;

class Api
{
public:
	Api(ICorrectSkew& corrector, const ILocateKeys& locator, IMessageSender& messenger, IStatusReporter& reporter, IStore& store, ISynchronize& sync, const Options& options);

	std::unique_ptr<Command> command(int id, const char* buff, unsigned size) const;
	std::unique_ptr<Command> command(const std::string& name, const std::unordered_map<std::string,std::string>& params) const;

	template <typename Req>
	std::unique_ptr<Command> command(const Req& req) const
	{
		std::unique_ptr<Command> cmd(command_impl(req._ID));
		if (!cmd)
			return NULL;
		*static_cast<Req*>(cmd->request()) = req;
		return cmd;
	}

protected:
	Command* command_impl(int id) const;
	std::unique_ptr<Command> command_impl(const std::string& name) const;

protected:
	std::unordered_map<std::string, int> _commands;

	ICorrectSkew& _corrector;
	const ILocateKeys& _locator;
	IMessageSender& _messenger;
	IStatusReporter& _reporter;
	IStore& _store;
	ISynchronize& _sync;
	const Options& _options;
};
}//namespace
