#pragma once

class ISwitchboard
{
public:
	~ISwitchboard() {}

	virtual void parse(const char* buffer, unsigned bytes) = 0;
};
