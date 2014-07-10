/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

// check drop list, drop file if we're supposed to.
#include "IAction.h"
#include <functional>
class DropParams;
class IDataStore;
class ILocateKeys;

class AckWriteAction : public IAction
{
public:
	AckWriteAction(IDataStore& store, const ILocateKeys& locator, std::function<void(DropParams)> onDrop=NULL);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

public:
	bool drop(const DropParams& params);

protected:
	IDataStore& _store;
	const ILocateKeys& _locator;
	std::function<void(DropParams)> _onDrop;

	std::string _filename;
	std::string _version;
	unsigned long long _size;
};
