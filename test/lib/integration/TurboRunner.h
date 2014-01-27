/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <string>

class TurboRunner
{
public:
	TurboRunner(short port, std::string programFlags="");
	~TurboRunner();

	short port() const;
	std::string dataChannel() const;

	void start();
	void stop();

	std::string query(std::string action) const;
	std::string local_list() const;
	bool waitForRunning(unsigned seconds=5) const;
	void createMemberFile(short firstPort, int firstUid=0, int members=1);

protected:
	short _port;
	std::string _dataChannel;
	std::string _programFlags;
	std::string _workingDir;
};
