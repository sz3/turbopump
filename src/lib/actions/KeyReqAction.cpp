/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "KeyReqAction.h"

#include "cohesion/ISynchronize.h"
using std::map;
using std::string;

KeyReqAction::KeyReqAction(const Peer& peer, ISynchronize& sync)
	: _peer(peer)
	, _sync(sync)
	, _first(0)
	, _last(0)
{
}

std::string KeyReqAction::name() const
{
	return "key-req";
}

bool KeyReqAction::run(const DataBuffer& data)
{
	_sync.pushKeyRange(_peer, _tree, _first, _last);
	return true;
}

void KeyReqAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator tree = params.find("tree");
	if (tree != params.end())
		_tree = tree->second;

	map<string,string>::const_iterator first = params.find("first");
	if (first != params.end())
		_first = std::stoull(first->second);

	_last = _first;
	map<string,string>::const_iterator last = params.find("last");
	if (last != params.end())
		_last = std::stoull(last->second);
}
