/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"
#include "cohesion/TreeId.h"
#include "membership/Peer.h"
class ISynchronize;

class KeyReqAction : public IAction
{
public:
	KeyReqAction(const Peer& peer, ISynchronize& sync);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	Peer _peer;
	ISynchronize& _sync;

	TreeId _tree;
	unsigned long long _first;
	unsigned long long _last;
};


