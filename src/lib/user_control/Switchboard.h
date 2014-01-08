/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <memory>
#include <string>

class DataBuffer;
class IAction;
class IByteStream;
class IDataStore;
class IHashRing;
class IMembership;
class IProcessState;
class TurboApi;

// "owns" the stream
class Switchboard
{
public:
	Switchboard(IByteStream& stream, IDataStore& dataStore, const IHashRing& ring, const IMembership& membership, const IProcessState& state, const TurboApi& callbacks);

	void run();

	bool parse(DataBuffer& data, std::unique_ptr<IAction>& action);
	std::unique_ptr<IAction> newAction(const std::string& actionName, const std::map<std::string,std::string>& params);

protected:
	IByteStream& _stream;
	IDataStore& _dataStore;
	const IHashRing& _ring;
	const IMembership& _membership;
	const IProcessState& _state;
	const TurboApi& _callbacks;
};

