/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

class Hash
{
public:
	Hash(std::string hash);
	static Hash compute(const std::string& input);

	std::string&& bytes();
	unsigned long long integer() const;
	size_t sizet() const;

protected:
	std::string _hash;
};
