/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serializers.h"
#include "common/turbopump_defaults.h"

namespace Turbopump {
class Write : public Request
{
public:
	static constexpr const char* _NAME = "write";
	static constexpr int _ID = 0;
	static constexpr int _INTERNAL_ID = 100;

public:
	std::string name;
	std::string version;
	short mirror = 0;
	short copies = DEFAULT_MIRRORS;
	unsigned long long offset = 0;
	std::string source;

	SERIALIZE(name, version, mirror, copies, offset, source);
};
}// namespace Turbopump
