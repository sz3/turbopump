/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "Request.h"
#include "common/serialize.h"
#include "deskew/TreeId.h"

namespace Turbopump {
class Sync : public Request, public TreeId
{
public:
	static constexpr const char* _NAME = "sync";
	static constexpr int _ID = 102;

public:
	// inherits fields from TreeId
	SERIALIZE(id, mirrors);
};
}//namespace
