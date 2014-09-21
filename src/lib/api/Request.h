#pragma once

#include <string>
#include <unordered_map>
namespace msgpack { struct object; }

namespace Turbopump {
class Request
{
public:
	virtual ~Request() {}

	virtual void load(const std::unordered_map<std::string,std::string>& params) = 0;
	virtual void msgpack_unpack(msgpack::object) = 0;
};
}// namespace Turbopump
