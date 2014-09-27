/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Command.h"

// TODO TODO TODO GOOD LORD TODO
// holding on to a real shared_ptr of a real Peer is ... not good.
// need to fix a number of things to make it go away though.

class InternalCommand : public Turbopump::Command
{
public:
	void setPeer(const std::shared_ptr<Peer>& peer)
	{
		_peer = peer;
	}

protected:
	std::shared_ptr<Peer> _peer;
};
