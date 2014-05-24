/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IUserPacketHandler.h"
#include <map>
#include <memory>
#include <string>

class DataBuffer;
class IDataStore;
class IHashRing;
class IHttpByteStream;
class IMembership;
class IKeyTabulator;
class IProcessState;
class TurboApi;

// "owns" the stream
class UserPacketHandler : public IUserPacketHandler
{
public:
	UserPacketHandler(IHttpByteStream& stream, IDataStore& dataStore, IHashRing& ring, IMembership& membership, IKeyTabulator& keyTabulator, const IProcessState& state, const TurboApi& callbacks);

	void run();
	void sendResponse(StatusCode status);

	std::unique_ptr<IAction> newAction(const std::string& actionName, const std::map<std::string,std::string>& params) const;

protected:
	IHttpByteStream& _stream;
	IDataStore& _dataStore;
	IHashRing& _ring;
	IMembership& _membership;
	IKeyTabulator& _keyTabulator;
	const IProcessState& _state;
	const TurboApi& _callbacks;
};

