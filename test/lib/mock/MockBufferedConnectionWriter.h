#pragma once

#include "wan_server/IBufferedConnectionWriter.h"
#include "util/CallHistory.h"

class MockBufferedConnectionWriter : public IBufferedConnectionWriter
{
public:
	unsigned capacity() const;
	int write(unsigned char virtid, const char* buffer, unsigned length);
	int flush();

public:
	unsigned _capacity = 1000;
	mutable CallHistory _history;
};
