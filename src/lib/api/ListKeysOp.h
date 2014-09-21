#pragma once

#include "ListKeys.h"

#include "Op.h"
#include <string>

class IByteStream;
class IDataStore;

class ListKeysOp : public Op
{
public:
	ListKeysOp(const IDataStore& dataStore, IByteStream& writer);

	bool run();
	Turbopump::Request* request();

protected:
	const IDataStore& _dataStore;
	IByteStream& _writer;

public:
	Turbopump::ListKeys params;
};
