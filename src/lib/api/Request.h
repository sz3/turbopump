/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>
#include <unordered_map>
namespace msgpack { namespace v2 { struct object; } }

namespace Turbopump {
class Request
{
public:
	virtual ~Request() {}

	virtual void load(const std::unordered_map<std::string,std::string>& params) = 0;
	virtual void save(std::unordered_map<std::string,std::string>& params) = 0;
	virtual void msgpack_unpack(msgpack::v2::object const&) = 0;
};
}// namespace Turbopump
