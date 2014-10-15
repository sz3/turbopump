/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <memory>
#include <string>
namespace Turbopump { class Api; class Command; }

class PeerPacketHandler : public IPeerPacketHandler
{
public:
	virtual std::shared_ptr<Turbopump::Command> command(unsigned cid, const char* buff, unsigned size) = 0;
	virtual void markFinished(const std::string& id) = 0;
};

