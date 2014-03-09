/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IAction.h"
#include "membership/Peer.h"
class ICorrectSkew;

class DemandWriteAction : public IAction
{
public:
	DemandWriteAction(const Peer& peer, ICorrectSkew& corrector);

	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

protected:
	Peer _peer;
	ICorrectSkew& _corrector;

	std::string _filename;
	std::string _version;
	std::string _source;
};
