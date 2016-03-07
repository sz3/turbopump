/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "IWatches.h"
#include <unordered_map>

struct watch_key_type
{
	std::string name;
	std::string id;

	bool operator==(const watch_key_type& other) const
	{
		return name == other.name && id == other.id;
	}
};

namespace std {
	template <>
	struct hash<watch_key_type>
	{
		std::size_t operator()(const watch_key_type& k) const
		{
			return std::hash<std::string>()(k.name);
		}
	};
}

class Watches : public IWatches
{
protected:
	using map_type = std::unordered_map<watch_key_type, std::function<void()>>;

public:
	void watch(const std::string& name, const std::string& id, std::function<void()> onNotify);
	bool unwatch(const std::string& name, const std::string& id);

	bool notify(const std::string& name) const;

protected:
	map_type _watches;
};

