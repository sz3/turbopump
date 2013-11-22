#include "Hash.h"

#include <cryptopp/tiger.h>
using std::string;

Hash::Hash(string hash)
	: _hash(std::move(hash))
{
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
