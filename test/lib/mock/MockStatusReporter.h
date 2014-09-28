/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "main/IStatusReporter.h"

class MockStatusReporter : public IStatusReporter
{
public:
	std::string status(const std::string& system) const
	{
		return system;
	}
};
