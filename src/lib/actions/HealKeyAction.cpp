/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "HealKeyAction.h"

#include "cohesion/ICorrectSkew.h"
#include "common/DataBuffer.h"
using std::map;
using std::string;

HealKeyAction::HealKeyAction(const Peer& peer, ICorrectSkew& corrector)
	: _peer(peer)
	, _corrector(corrector)
	, _key(0)
{
}

std::string HealKeyAction::name() const
{
	return "heal-key";
}

bool HealKeyAction::run(const DataBuffer& data)
{
	_corrector.healKey(_peer, _tree, _key);
	return true;
}

void HealKeyAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("tree");
	if (it != params.end())
		_tree.id = it->second;

	// TODO: stoul and friends throw. Wrap them with something that doesn't.
	it = params.find("n");
	if (it != params.end())
		_tree.mirrors = std::stoul(it->second);

	it = params.find("key");
	if (it != params.end())
		_key = std::stoull(it->second);
}
