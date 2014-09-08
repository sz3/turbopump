/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Hash.h"

#include "serialize/base64.h"
#include "crypto/tiger_hash.h"
using std::string;

namespace {
	std::function<std::string(const std::string&)> _hasher = [] (const string& in) {
		tiger_hash hasher;
		return hasher(in);
	};
}

void Hash::init(std::function<std::string(const std::string&)> hasher)
{
	_hasher = hasher;
}

Hash::Hash()
{
}

Hash::Hash(const string& input)
	: _hash(_hasher(input))
{
}

Hash& Hash::fromBase64(const string& encoded)
{
	_hash = base64::decode(encoded);
	return *this;
}

std::string Hash::base64() const
{
	return base64::encode((const unsigned char*)_hash.data(), _hash.size());
}

unsigned long long Hash::integer() const
{
	/*unsigned long long result = 0;
	unsigned char* arr = (unsigned char*)&result;
	unsigned len = sizeof(unsigned long long);
	for (int i = 0; i < len; ++i)
		arr[i] |= _hash.data()[i];
	return result;*/

	return *(unsigned long long*)_hash.data();
}
