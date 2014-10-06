/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serializers.h"

namespace Turbopump {
class Read : public Request
{
public:
	static constexpr const char* _NAME = "read";
	static constexpr int _ID = 2;

public:
	std::string name;
	std::string version;

	SERIALIZE(name, version);
};
}//namespace
