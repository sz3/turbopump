/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serializers.h"

namespace Turbopump {
class OfferWrite : public Request
{
public:
	static constexpr const char* _NAME = "offer-write";
	static constexpr int _ID = 105;

public:
	std::string name;
	std::string version;
	std::string source;

	SERIALIZE(name, version, source);
};
}//namespace
