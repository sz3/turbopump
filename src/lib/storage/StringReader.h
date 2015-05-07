/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
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
		unsigned chunk = _data.size();
		if (chunk == 0)
			return 0;

		if (_chunkSize != 0 && _chunkSize < chunk)
			chunk = _chunkSize;
		int res = sink.write(_data.data(), chunk);
		_data = _data.substr(chunk);
		return res;
	}

protected:
	std::string _data;
	unsigned _chunkSize;
};
