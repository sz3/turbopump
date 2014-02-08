/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"
#include <functional>
class DropParams;
class IDataStore;
class ILocateKeys;

class DropAction : public IAction
{
public:
	DropAction(IDataStore& dataStore, const ILocateKeys& locator, std::function<void(DropParams)> onDrop=NULL);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	IDataStore& _dataStore;
	const ILocateKeys& _locator;
	std::function<void(DropParams)> _onDrop;

	std::string _filename;
};
