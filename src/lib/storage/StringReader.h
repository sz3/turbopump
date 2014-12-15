#pragma once

#include "IReader.h"
#include "socket/IByteStream.h"

class StringReader : public IReader
{
public:
	StringReader(const std::string& data)
		: _data(data)
	{}

	bool good() const
	{
		return true;
	}

	unsigned long long size() const
	{
		return _data.size();
	}

	bool setPosition(unsigned long long offset)
	{
		return true;
	}

	int stream(IByteStream& sink)
	{
		int res = sink.write(_data.data(), _data.size());
		_data.clear();
		return res;
	}

protected:
	std::string _data;
};
