#pragma once

#include "IReader.h"
#include "socket/IByteStream.h"

class StringReader : public IReader
{
public:
	StringReader(const std::string& data, unsigned chunkSize=0)
		: _data(data)
		, _chunkSize(chunkSize)
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
		unsigned chunk = std::min(_chunkSize, _data.size());
		int res = sink.write(_data.data(), chunk);
		_data = _data.substr(chunk);
		return res;
	}

protected:
	std::string _data;
	unsigned _chunkSize;
};
