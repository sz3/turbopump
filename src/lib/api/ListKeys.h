/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serialize.h"

namespace Turbopump {
class ListKeys : public Request
{
public:
	static constexpr const char* _NAME = "list-keys";
	static constexpr int _ID = 3;

public:
	bool all = false;
	bool deleted = false;

	SERIALIZE(all, deleted);
};
}//namespace
