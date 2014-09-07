/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <functional>
#include <string>

class Hash
{
public:
	static void init(std::function<std::string(const std::string&)> hasher);

public:
	Hash(std::string hash);
	static Hash fromBase64(const std::string& encoded);
	static Hash compute(const std::string& input);

	std::string&& bytes();
	std::string base64() const;
	unsigned long long integer() const;
	size_t sizet() const;

protected:
	std::string _hash;
};
