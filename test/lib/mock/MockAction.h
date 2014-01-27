/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "actions/IAction.h"
#include "util/CallHistory.h"

class MockAction : public IAction
{
public:
	std::string name() const;
	bool run(const DataBuffer& data);
	void setParams(const std::map<std::string,std::string>& params);

public:
	CallHistory _history;
};
