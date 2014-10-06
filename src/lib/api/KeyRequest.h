/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serializers.h"
#include "deskew/TreeId.h"

namespace Turbopump {
class KeyRequest : public Request, public TreeId
{
public:
	static constexpr const char* _NAME = "key-req";
	static constexpr int _ID = 103;

public:
	unsigned long long first;
	unsigned long long last;

	// inherits fields from TreeId
	SERIALIZE(id, mirrors, first, last);
};
}//namespace
