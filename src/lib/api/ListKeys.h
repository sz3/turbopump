#pragma once

#include "Request.h"
#include "common/serialize.h"

namespace Turbopump {
class ListKeys : public Request
{
public:
	static constexpr const char* name = "list-keys";

public:
	ListKeys() {}

	ListKeys(bool all, bool deleted)
		: all(all)
		, deleted(deleted)
	{}

public:
	bool all = false;
	bool deleted = false;

	SERIALIZE(all, deleted);
};
}// namespace Turbopump
