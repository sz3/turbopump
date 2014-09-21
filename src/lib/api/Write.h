#pragma once

#include "Request.h"
#include "common/serialize.h"
#include "common/turbopump_defaults.h"

namespace Turbopump {
class Write : public Request
{
public:
	static constexpr const char* NAME = "write";

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
