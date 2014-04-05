/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DeleteAction.h"

#include "common/DataBuffer.h"
#include "common/VectorClock.h"
using std::map;
using std::string;

DeleteAction::DeleteAction(std::unique_ptr<IAction>&& writeAction)
	: _writeAction(std::move(writeAction))
{
}

std::string DeleteAction::name() const
{
	return "delete";
}

bool DeleteAction::run(const DataBuffer& data)
{
	VectorClock version;
	version.fromString(_version);
	version.increment("delete");

	map<string,string> params;
	params["name"] = _name;
	params["v"] = version.toString();
	_writeAction->setParams(params);

	string timestamp("timestamp");
	DataBuffer buff(timestamp.data(), timestamp.size());
	return _writeAction->run(buff);
}

void DeleteAction::setParams(const map<string,string>& params)
{
	map<string,string>::const_iterator it = params.find("name");
	if (it != params.end())
		_name = it->second;

	it = params.find("version");
	if (it != params.end())
		_version = it->second;
}
