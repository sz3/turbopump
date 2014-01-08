/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "main/IProcessState.h"

class MockProcessState : public IProcessState
{
public:
	std::string summary() const;

public:
	std::string _summary;
};
