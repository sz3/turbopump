/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "api/Options.h"

namespace Turbopump { class Interface; }

class ComputedOptions : public Turbopump::Options
{
public:
	ComputedOptions(const Turbopump::Options& opts, const Turbopump::Interface& turbopump);
};
