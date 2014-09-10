/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockLocateKeys.h"

#include "serialize/StringUtil.h"

MockLocateKeys::MockLocateKeys()
	: _mine(true)
{
}

std::vector<std::string> MockLocateKeys::locations(const std::string& name, unsigned mirrors) const
{
	_history.call("locations", name, mirrors);
	return _locations;
}

std::string MockLocateKeys::section(const std::string& name) const
{
	_history.call("section", name);
	return _locations.empty()? "" : _locations.front();
}

bool MockLocateKeys::containsSelf(const std::vector<std::string>& locs) const
{
	_history.call("containsSelf", StringUtil::join(locs));
	return _mine;
}

bool MockLocateKeys::keyIsMine(const std::string& name, unsigned mirrors) const
{
	_history.call("keyIsMine", name, mirrors);
	return _mine;
}

bool MockLocateKeys::sectionIsMine(const std::string& id, unsigned mirrors) const
{
	_history.call("sectionIsMine", id, mirrors);
	return _mine;
}
