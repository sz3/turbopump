/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "LocalListAction.h"

#include "common/turbopump_defaults.h"
#include "data_store/IDataStore.h"
using std::map;
using std::string;

LocalListAction::LocalListAction(const IDataStore& dataStore, IByteStream& writer)
	: _dataStore(dataStore)
	, _writer(writer)
	, _showAll(false)
{
}

std::string LocalListAction::name() const
{
	return "local_list";
}

bool LocalListAction::run(const DataBuffer& data)
{
	string excludes = _showAll? "" : MEMBERSHIP_FILE_PREFIX;
	_dataStore.report(_writer, excludes);
	return true;
}

void LocalListAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("all");
	if (it != params.end())
		_showAll = true;
}
