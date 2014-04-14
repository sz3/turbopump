/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"
class IByteStream;
class IDataStore;

class LocalListAction : public IAction
{
public:
	LocalListAction(const IDataStore& dataStore, IByteStream& writer);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	const IDataStore& _dataStore;
	IByteStream& _writer;
	bool _showAll;
	bool _showDeleted;
};

