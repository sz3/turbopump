/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serializers.h"

namespace Turbopump {
class RemovePeer : public Request
{
public:
	static constexpr const char* _NAME = "remove-peer";
	static constexpr int _ID = 7;

public:
	std::string uid;

	SERIALIZE(uid);
};
}//namespace
