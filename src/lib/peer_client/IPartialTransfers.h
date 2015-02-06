/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <functional>

class IPartialTransfers
{
public:
	virtual ~IPartialTransfers() {}

	virtual void add(int id, const std::function<bool()>& fun) = 0;
	virtual bool run(int id) = 0;
};
