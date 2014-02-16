/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

// singleton
class MyMemberId
{
public:
	MyMemberId();
	MyMemberId(const std::string& id);

	const std::string& str() const;
	operator const std::string&() const;

protected:
	class MyMemberIdImpl;
	MyMemberIdImpl& _pimpl;
};
