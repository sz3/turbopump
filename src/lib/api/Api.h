/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

// will probably make Api the interface, and name the impl something else.
// but for now just make it work.
#include "Command.h"
#include "data_store/IDataStoreReader.h"
#include <functional>
#include <memory>
#include <unordered_map>

class DataBuffer;
class IByteStream;
class IConsistentHashRing;
class IDataStore;
class IHttpByteStream;
class ILocateKeys;
class IKeyTabulator;
class IMembership;
class IProcessState;
class TurboApi;
class WriteInstructions;

namespace Turbopump {
class Options;

class Api
{
public:
	Api(IDataStore& dataStore, const ILocateKeys& locator, IByteStream& writer, const Options& options);

	std::unique_ptr<Command> command(int id, const DataBuffer& buffer) const;
	std::unique_ptr<Command> command(const std::string& name) const;
	std::unique_ptr<Command> command(const std::string& name, const std::unordered_map<std::string,std::string>& params) const;

protected:
	Command* command(int id) const;

protected:
	std::unordered_map<std::string, int> _commands;

	IDataStore& _dataStore;
	const ILocateKeys& _locator;
	IByteStream& _writer;
	const Options& _options;
};
}//namespace
