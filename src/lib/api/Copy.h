/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Write.h"
#include "common/serializers.h"
#include "common/turbopump_defaults.h"

namespace Turbopump {
class Copy : public Write
{
public:
	static constexpr const char* _NAME = "copy";
	static constexpr int _ID = 10;

public:
	std::string path;

	SERIALIZE(name, version, mirror, copies, offset, source, path);
};
}// namespace Turbopump
