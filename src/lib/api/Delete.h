/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serialize.h"

namespace Turbopump {
class Delete : public Request
{
public:
	static constexpr const char* _NAME = "delete";
	static constexpr int _ID = 5;

public:
	std::string name;
	std::string version;

	SERIALIZE(name, version);
};
}//namespace
