/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MockAction.h"

#include "common/DataBuffer.h"
#include "serialize/StringUtil.h"
using std::map;
using std::string;

namespace {
	template <class T1, class T2>
	std::ostream& operator<<(std::ostream& outstream, const std::pair<T1,T2>& pear)
	{
		outstream << pear.first << "=" << pear.second;
		return outstream;
	}
}

std::string MockAction::name() const
{
	return "mock";
}

bool MockAction::run(const DataBuffer& data)
{
	_history.call("run", data.str());
	return true;
}

void MockAction::setParams(const std::map<std::string,std::string>& params)
{
	_history.call("setParams", StringUtil::join(params));
}
