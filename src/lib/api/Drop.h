/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serialize.h"
#include "common/turbopump_defaults.h"

namespace Turbopump {
class Drop : public Request
{
public:
	static constexpr const char* _NAME = "drop";
	static constexpr int _ID = 4;

public:
	std::string name;
	short copies = DEFAULT_MIRRORS;

	SERIALIZE(name, copies);
};
}//namespace
