/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "Hash.h"

#include <cryptopp/tiger.h>
#include "base64.h"
using std::string;

Hash::Hash(string hash)
	: _hash(std::move(hash))
{
}

Hash Hash::fromBase64(const string& encoded)
{
	return Hash(base64_decode(encoded));
}

Hash Hash::compute(const string& input)
{
	string hash;
	hash.resize(CryptoPP::Tiger::DIGESTSIZE);
	CryptoPP::Tiger().CalculateDigest((unsigned char*)(&hash[0]), (const unsigned char*)input.data(), input.size());
	return Hash(hash);
}

std::string&& Hash::bytes()
{
	return std::move(_hash);
}

std::string Hash::base64() const
{
	return base64_encode((const unsigned char*)_hash.data(), _hash.size());
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

size_t Hash::sizet() const
{
	return *(size_t*)_hash.data();
}
