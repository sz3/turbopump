/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "KeyReqAction.h"

#include "deskew/ICorrectSkew.h"
using std::map;
using std::string;

KeyReqAction::KeyReqAction(const Peer& peer, ICorrectSkew& corrector)
	: _peer(peer)
	, _corrector(corrector)
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
	_corrector.pushKeyRange(_peer, _tree, _first, _last);
	return true;
}

void KeyReqAction::setParams(const std::map<std::string,std::string>& params)
{
	map<string,string>::const_iterator it = params.find("tree");
	if (it != params.end())
		_tree.id = it->second;

	it = params.find("n");
	if (it != params.end())
		_tree.mirrors = std::stoul(it->second);

	it = params.find("first");
	if (it != params.end())
		_first = std::stoull(it->second);

	_last = _first;
	it = params.find("last");
	if (it != params.end())
		_last = std::stoull(it->second);
}
