/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#pragma once

#include <map>
#include <set>
#include <string>

class TurboRunner
{
public:
	TurboRunner(short port, std::string programFlags="");
	~TurboRunner();

	short port() const;
	std::string dataChannel() const;

	void start();
	bool stop(unsigned retries=1);

	std::string query(std::string action, std::string params="") const;
	std::string post(std::string action, std::string params="", std::string body="") const;
	std::string local_list(std::string params="") const; // old
	std::map<std::string,std::string> list_keys(std::string params="") const;
	std::set<std::string> membership() const;

	bool waitForRunning(unsigned seconds=5) const;
	bool waitForState(std::string state, unsigned seconds=5) const;

	std::string write(std::string name, std::string data, std::string params="");
	static std::string headerForWrite(std::string name, unsigned contentLength, std::string params="");
	static std::string headerForRead(std::string name, std::string params="");

	void createMemberFile(short firstPort, int firstUid=0, int members=1);

protected:
	short _port;
	std::string _dataChannel;
	std::string _programFlags;
	std::string _workingDir;
};
