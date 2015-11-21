/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once
#include "RemovePeer.h"

#include "Command.h"
namespace Turbopump { class Api; }

class RemovePeerCommand : public Turbopump::Command
{
public:
	RemovePeerCommand(const Turbopump::Api& api);

	bool run(const char* buff=NULL, unsigned size=0);
	Turbopump::Request* request();

protected:
	const Turbopump::Api& _api;

public:
	Turbopump::RemovePeer params;
};
