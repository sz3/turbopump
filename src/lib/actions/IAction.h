/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <string>

class DataBuffer;

class IAction
{
public:
	virtual ~IAction() {}

	virtual std::string name() const = 0;
	virtual bool run(const DataBuffer& data) = 0;
	virtual void setParams(const std::map<std::string,std::string>& params) {}

	virtual bool multiPacket() const { return false; }
	virtual bool good() const { return true; }

protected:
};
