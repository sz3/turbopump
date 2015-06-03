/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "MyMemberId.h"

#include "serialize/base64.h"
#include <random>
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

MyMemberId MyMemberId::generate()
{
	// basic for now, just use entropy directly.
	// eventually there will be crypto involved...
	// and this might need to be split out into a different class to avoid linker stupidity.
	constexpr int ID_BYTES = 16;

	std::random_device rd;
	std::string raw_id;
	for (int bits = 0; bits < ID_BYTES; bits += sizeof(unsigned int))
	{
		unsigned int entropy = rd();
		raw_id += std::string( reinterpret_cast<char*>(&entropy), sizeof(unsigned int) );
	}
	return MyMemberId(base64::encode(raw_id));
}

const string& MyMemberId::str() const
{
	return _pimpl.id();
}

MyMemberId::operator const string&() const
{
	return str();
}
