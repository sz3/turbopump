/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Hash.h"

#include "serialize/base64.h"
#include "crypto/tiger_hash.h"
using std::string;

namespace {
	std::function<std::string(const std::string&)> _hasher = [] (const string& in) {
		turbo::tiger_hash hasher;
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

Hash::Hash(const char* input)
	: Hash(string(input))
{
}

Hash::Hash(const string& input)
	: _hash(_hasher(input))
{
}

Hash& Hash::fromHash(const string& hash)
{
	_hash = hash;
	return *this;
}

Hash& Hash::fromBase64(const string& encoded)
{
	_hash = base64::decode(encoded);
	return *this;
}

std::string Hash::base64() const
{
	return base64::encode(_hash);
}

const std::string& Hash::str() const
{
	return _hash;
}

unsigned long long Hash::integer() const
{
	/*unsigned long long result = 0;
	unsigned char* arr = reinterpret_cast<unsigned char*>(&result);
	unsigned len = sizeof(unsigned long long);
	for (int i = 0; i < len; ++i)
		arr[i] |= _hash.data()[i];
	return result;*/

	return *reinterpret_cast<const unsigned long long*>(_hash.data());
}
