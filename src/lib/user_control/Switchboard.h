#pragma once

#include <map>
#include <memory>
#include <string>

class Callbacks;
class DataBuffer;
class IAction;
class IByteStream;
class IDataStore;
class IMembership;
class LocalDataStore;

// "owns" the stream
class Switchboard
{
public:
	Switchboard(IByteStream& stream, IDataStore& dataStore, const LocalDataStore& localDataStore, const IMembership& membership, const Callbacks& callbacks);

	void run();

	bool parse(DataBuffer& data, std::unique_ptr<IAction>& action);
	std::unique_ptr<IAction> newAction(const std::string& actionName, const std::map<std::string,std::string>& params);

protected:
	IByteStream& _stream;
	IDataStore& _dataStore;

	const LocalDataStore& _localDataStore;
	const IMembership& _membership;
	const Callbacks& _callbacks;
};

