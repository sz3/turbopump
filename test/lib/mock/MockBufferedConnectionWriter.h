/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include "peer_server/IBufferedConnectionWriter.h"
#include "util/CallHistory.h"

class MockBufferedConnectionWriter : public IBufferedConnectionWriter
{
public:
	unsigned capacity() const;
	int write(unsigned char virtid, const char* buffer, unsigned length, bool blocking);
	bool flush(bool blocking);
	bool close();

	std::string target() const;

public:
	unsigned _capacity = 1000;
	mutable CallHistory _history;
};
