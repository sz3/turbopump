/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <string>

class DataBuffer;

class ActionParser
{
public:
	ActionParser();
	bool parse(DataBuffer& buffer);

	const std::string& action() const;
	const std::map<std::string,std::string>& params() const;

	const std::string& lastError() const;

protected:
	bool setError(const std::string& errormsg);

protected:
	std::string _action;
	std::map<std::string,std::string> _params;

	std::string _lastError;
};
