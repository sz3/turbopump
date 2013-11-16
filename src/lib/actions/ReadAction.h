#pragma once

#include "IAction.h"

class IByteStream;
class IDataStore;

// TODO: how to rate limit UDP?
//  Answer: have the protocol handle it, e.g. UDT
class ReadAction : public IAction
{
public:
	ReadAction(IDataStore& dataStore, IByteStream& writer);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	IDataStore& _dataStore;
	IByteStream& _writer;

	std::string _filename;
};

