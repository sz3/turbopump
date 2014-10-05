#pragma once

#include "msgpack.hpp"
#include "serialize/define_serialize_map.h"

#define SERIALIZE(...) DEFINE_SERIALIZE_MAP(__VA_ARGS__); MSGPACK_DEFINE(__VA_ARGS__);
