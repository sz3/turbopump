#include "ActionParser.h"

#include "DataBuffer.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using std::string;
using std::vector;

ActionParser::ActionParser()
{
}

bool ActionParser::parse(DataBuffer& buff)
{
	const char* buffer = buff.buffer();
	unsigned bytes = buff.size();

	const char* actionEnd = std::char_traits<char>::find(buffer, bytes, '|');
	if (actionEnd == NULL)
		return setError("no action specified");

	_action = string(buffer, actionEnd);
	bytes -= (actionEnd+1-buffer);

	const char* paramEnd = std::char_traits<char>::find(actionEnd+1, bytes, '|');
	if (paramEnd == NULL)
		return setError("no param end token specified for action " + _action);
	bytes -= (paramEnd-actionEnd);

	std::stringstream ss( string(actionEnd+1, paramEnd) );
	vector<string> tokens;
	string temp;
	while (ss >> temp)
		tokens.push_back(temp);

	for (vector<string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
	{
		size_t pos = it->find('=');
		if (pos == string::npos)
			continue;
		_params[string(*it, 0, pos)] = string(*it, pos+1);
	}

	buff.skip(paramEnd+1-buffer);
	return true;
}

const std::string& ActionParser::action() const
{
	return _action;
}

const std::map<std::string,std::string>& ActionParser::params() const
{
	return _params;
}

const std::string& ActionParser::lastError() const
{
	return _lastError;
}

bool ActionParser::setError(const std::string& errormsg)
{
	_lastError = errormsg;
	return false;
}
