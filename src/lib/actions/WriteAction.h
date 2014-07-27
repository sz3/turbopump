/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"
#include "actions/WriteParams.h"
#include "data_store/IDataStoreReader.h"
#include "data_store/IDataStoreWriter.h"
#include <functional>

class IDataStore;

class WriteAction : public IAction
{
public:
	// instead of passing in a function, dedicate an interface to predefined functions, and have the params select from it?
	WriteAction(IDataStore& dataStore, std::function<void(WriteParams&, IDataStoreReader::ptr)> onCommit=NULL);
	~WriteAction();

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

	virtual IDataStoreWriter::ptr open(const WriteParams& params);

	bool multiPacket() const;
	bool good() const;
	bool finished() const;

protected:
	bool flush();
	bool commit();

protected:
	IDataStore& _dataStore;
	std::function<void(WriteParams&, IDataStoreReader::ptr)> _onCommit;
	bool _started;
	bool _finished;
	unsigned _bytesSinceLastFlush;

	WriteParams _params;
	IDataStoreWriter::ptr _writer;
};
