/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serializers.h"

namespace Turbopump {
class DemandWrite : public Request
{
public:
	static constexpr const char* _NAME = "demand-write";
	static constexpr int _ID = 106;

public:
	std::string name;
	std::string version;
	std::string source;
	uint64_t offset = 0;

	SERIALIZE(name, version, source, offset);
};
}//namespace
