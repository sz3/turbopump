/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "DemandWriteAction.h"

#include "WriteParams.h"
#include "cohesion/ICorrectSkew.h"
#include "common/DataBuffer.h"

using std::map;
using std::string;

DemandWriteAction::DemandWriteAction(const Peer& peer, ICorrectSkew& corrector)
	: _peer(peer)
	, _corrector(corrector)
{
}

std::string DemandWriteAction::name() const
{
	return "demand-write";
}

bool DemandWriteAction::run(const DataBuffer& data)
{
	if (_filename.empty())
		return false;

	_corrector.sendKey(_peer, _filename, _version, _source);
	return true;
}

void DemandWriteAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("name");
	if (it != params.end())
		_filename = it->second;

	it = params.find("v");
	if (it != params.end())
		_version = it->second;

	it = params.find("source");
	if (it != params.end())
		_source = it->second;
}
