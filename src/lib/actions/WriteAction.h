#pragma once

#include "IAction.h"
#include "common/KeyMetadata.h"
#include "data_store/IDataStoreReader.h"
#include "data_store/IDataStoreWriter.h"
#include <functional>

class IDataStore;

class WriteAction : public IAction
{
public:
	// instead of passing in a function, dedicate an interface to predefined functions, and have the params select from it?
	WriteAction(IDataStore& dataStore, std::function<void(KeyMetadata, IDataStoreReader::ptr)> onCommit=NULL);
	~WriteAction();

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

	bool multiPacket() const;
	bool good() const;
	bool finished() const;

protected:
	bool commit();

protected:
	IDataStore& _dataStore;
	std::function<void(KeyMetadata, IDataStoreReader::ptr)> _onCommit;
	bool _started;
	bool _finished;

	KeyMetadata _metadata;
	IDataStoreWriter::ptr _writer;
};
