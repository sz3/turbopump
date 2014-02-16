/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MyMemberId.h"

#include <string>
using std::string;

class MyMemberId::MyMemberIdImpl
{
public:
	static MyMemberIdImpl* instance()
	{
		static MyMemberIdImpl* id = new MyMemberIdImpl;
		return id;
	}

	const string& id() const
	{
		return _id;
	}

	void setId(const string& id)
	{
		_id = id;
	}

protected:
	string _id;
};

MyMemberId::MyMemberId()
	: _pimpl(*MyMemberIdImpl::instance())
{
}

MyMemberId::MyMemberId(const string& id)
	: _pimpl(*MyMemberIdImpl::instance())
{
	_pimpl.setId(id);
}

const string& MyMemberId::str() const
{
	return _pimpl.id();
}

MyMemberId::operator const string&() const
{
	return str();
}
