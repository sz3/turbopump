/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockRequestPacker.h"

#include "DummyTurbopumpApi.h"
#include "api/Request.h"
#include "serialize/StringUtil.h"
#include "msgpack.hpp"
#include <map>
#include <unordered_map>
using std::string;

namespace {
	template <class T1, class T2>
	std::ostream& operator<<(std::ostream& outstream, const std::pair<T1,T2>& pear)
	{
		outstream << pear.first << "=" << pear.second;
		return outstream;
	}
}
#include "serialize/str_join.h"

std::string MockRequestPacker::package(unsigned char id, const char* packet, unsigned size) const
{
	_history.call("package", (unsigned)id);

	// we're just a mock, so deserialize packet so tests can validate parameters normally.
	DummyTurbopumpApi api;
	std::unique_ptr<Turbopump::Command> cmd = api.command(id, packet, size);

	std::unordered_map<string,string> map;
	cmd->request()->save(map);

	std::map<string,string> sorted;
	for (auto it = map.begin(); it != map.end(); ++it)
		sorted[it->first] = it->second;
	return "{" + StringUtil::str((unsigned)id) + " " + turbo::str::join(sorted) + "}";
}
