/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serializers.h"

namespace Turbopump {
class Status : public Request
{
public:
	static constexpr const char* _NAME = "status";
	static constexpr int _ID = 107;
	static constexpr int _ID2 = 108;
	static constexpr int _ID3 = 109;

public:
	std::string view;

	SERIALIZE(view);
};
}//namespace
