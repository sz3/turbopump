/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

class TurboRunner
{
public:
	TurboRunner(short port, std::string dataChannel);
	~TurboRunner();

	short port() const;
	std::string dataChannel() const;

	void start();
	void stop();

	std::string query(std::string action) const;
	bool waitForRunning(unsigned seconds=5) const;
	static void createMemberFile(int members);

protected:
	short _port;
	std::string _dataChannel;
};
