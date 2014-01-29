/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "AddPeerAction.h"

#include "cohesion/IKeyTabulator.h"
#include "common/DataBuffer.h"
#include "common/turbopump_defaults.h"
#include "consistent_hashing/IHashRing.h"
#include "membership/IMembership.h"
using std::map;
using std::string;

AddPeerAction::AddPeerAction(std::unique_ptr<IAction>&& writeAction)
	: _writeAction(std::move(writeAction))
{
}

std::string AddPeerAction::name() const
{
	return "add_peer";
}

bool AddPeerAction::run(const DataBuffer& data)
{
	map<string,string> params;
	params["name"] = MEMBERSHIP_FILE_PREFIX + _uid;
	params["n"] = "0";
	_writeAction->setParams(params);

	DataBuffer buff(_ip.data(), _ip.size());
	return _writeAction->run(buff);
}

void AddPeerAction::setParams(const map<string,string>& params)
{
	map<string,string>::const_iterator it = params.find("uid");
	if (it != params.end())
		_uid = it->second;

	it = params.find("ip");
	if (it != params.end())
		_ip = it->second;
}
