/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serialize.h"
#include "deskew/TreeId.h"

namespace Turbopump {
class HealKey : public Request, public TreeId
{
public:
	static constexpr const char* _NAME = "heal-key";
	static constexpr int _ID = 104;

public:
	unsigned long long key;

	// inherits fields from TreeId
	SERIALIZE(id, mirrors, key);
};
}//namespace
