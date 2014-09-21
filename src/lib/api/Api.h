#pragma once

// will probably make Api the interface, and name the impl something else.
// but for now just make it work.
#include "Op.h"
#include <functional>
#include <memory>
#include <unordered_map>

class DataBuffer;
class IByteStream;
class IConsistentHashRing;
class IDataStore;
class IHttpByteStream;
class IKeyTabulator;
class IMembership;
class IProcessState;

class Api
{
public:
	Api(const IDataStore& dataStore, IByteStream& writer);

	std::unique_ptr<Op> op(const std::string& command) const;
	std::unique_ptr<Op> op(const std::string& command, const DataBuffer& buffer) const;
	std::unique_ptr<Op> op(const std::string& command, const std::unordered_map<std::string,std::string>& params) const;

protected:
	std::unordered_map<std::string, std::function<Op*()>> _ops;
};
