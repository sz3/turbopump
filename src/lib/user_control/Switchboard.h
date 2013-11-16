#pragma once

#include "ISwitchboard.h"
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

// "owns" the BlockingReader (because of RAII he *technically* doesn't, but...)
// and manages his own thread.
class Switchboard : public ISwitchboard
{
public:
	// should take a blockingWriter as well
	Switchboard(IByteStream& stream, IDataStore& dataStore, const LocalDataStore& localDataStore, const IMembership& membership, const Callbacks& callbacks);

	void run();

	void parse(const char* buffer, unsigned size);
	std::shared_ptr<IAction> newAction(const std::string& actionName, const std::map<std::string,std::string>& params);

protected:
	IByteStream& _stream;
	IDataStore& _dataStore;

	const LocalDataStore& _localDataStore;
	const IMembership& _membership;
	const Callbacks& _callbacks;
};

