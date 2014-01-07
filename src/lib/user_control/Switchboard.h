/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <memory>
#include <string>

class DataBuffer;
class IAction;
class IByteStream;
class IDataStore;
class IMembership;
class TurboApi;

// "owns" the stream
class Switchboard
{
public:
	Switchboard(IByteStream& stream, IDataStore& dataStore, const IMembership& membership, const TurboApi& callbacks);

	void run();

	bool parse(DataBuffer& data, std::unique_ptr<IAction>& action);
	std::unique_ptr<IAction> newAction(const std::string& actionName, const std::map<std::string,std::string>& params);

protected:
	IByteStream& _stream;
	IDataStore& _dataStore;
	const IMembership& _membership;
	const TurboApi& _callbacks;
};

