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

	static void createMemberFile(int members);

protected:
	short _port;
	std::string _dataChannel;
};
