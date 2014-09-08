/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <functional>
#include <string>

class Hash
{
public:
	static void init(std::function<std::string(const std::string&)> hasher);

public:
	Hash();
	Hash(const std::string& input);
	Hash& fromBase64(const std::string& encoded);

	std::string base64() const;
	unsigned long long integer() const;
	size_t sizet() const;

protected:
	std::string _hash;
};
