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
		return sink.write(_data.data(), _data.size());
	}

protected:
	std::string _data;
};
