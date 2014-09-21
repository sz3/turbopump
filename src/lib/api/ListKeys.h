#pragma once

#include "Request.h"
#include "common/serialize.h"

namespace Turbopump {
class ListKeys : public Request
{
public:
	static constexpr const char* NAME = "list-keys";

public:
	bool all = false;
	bool deleted = false;

	SERIALIZE(all, deleted);
};
}// namespace Turbopump
