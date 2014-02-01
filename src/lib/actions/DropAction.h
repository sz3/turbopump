/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"
#include <functional>
class DropParams;
class IDataStore;
class IHashRing;
class IMembership;

class DropAction : public IAction
{
public:
	DropAction(IDataStore& dataStore, const IHashRing& ring, const IMembership& membership, std::function<void(DropParams)> onDrop=NULL);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	IDataStore& _dataStore;
	const IHashRing& _ring;
	const IMembership& _membership;
	std::function<void(DropParams)> _onDrop;

	std::string _filename;
};
