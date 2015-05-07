/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Options.h"

namespace Turbopump { class Interface; }

class BuildCallbacks
{
public:
	BuildCallbacks(Turbopump::Options& opts);

	void build(const Turbopump::Interface& turbopump);

protected:
	Turbopump::Options& _opts;
};

