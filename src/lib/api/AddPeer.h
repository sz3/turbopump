/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serializers.h"

namespace Turbopump {
class AddPeer : public Request
{
public:
	static constexpr const char* _NAME = "add-peer";
	static constexpr int _ID = 6;

public:
	std::string uid;
	std::string ip;

	SERIALIZE(uid, ip);
};
}//namespace
