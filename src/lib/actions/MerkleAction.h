#pragma once

#include "IAction.h"
#include "membership/Peer.h"
class ISynchronize;

class MerkleAction : public IAction
{
public:
	MerkleAction(const Peer& peer, ISynchronize& sync);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	Peer _peer;
	ISynchronize& _sync;
};
