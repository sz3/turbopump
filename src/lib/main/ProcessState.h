/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IProcessState.h"

class ProcessState : public IProcessState
{
public:
	void starting();
	void running();
	void stopping();

	std::string summary() const;

protected:
	std::string _summary;
};
