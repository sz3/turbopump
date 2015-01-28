/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <functional>
class ISocketWriter;

class IPartialTransfers
{
public:
	virtual ~IPartialTransfers() {}

	virtual void add(ISocketWriter& writer, const std::function<bool()>& fun) = 0;
	virtual bool run(ISocketWriter& writer) = 0;
};
