/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "ProcessState.h"

void ProcessState::starting()
{
	_summary = "starting";
}

void ProcessState::running()
{
	_summary = "running";
}

void ProcessState::stopping()
{
	_summary = "stopping";
}

std::string ProcessState::summary() const
{
	return _summary;
}
