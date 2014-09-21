#pragma once

#include "msgpack.hpp"
#include "serialize/define_load_map.h"

#define SERIALIZE(...) DEFINE_LOAD_MAP(__VA_ARGS__); MSGPACK_DEFINE(__VA_ARGS__);
