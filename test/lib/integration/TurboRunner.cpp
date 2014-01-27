/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboRunner.h"
#include "command_line/CommandLine.h"
#include "membership/Membership.h"
#include "serialize/StringUtil.h"
#include "time/Timer.h"

#include "boost/filesystem.hpp"
#include <iostream>
#include <vector>
using std::string;
using std::vector;

namespace {
	string exePath = string(TURBOPUMP_PROJECT_ROOT) + "/build/src/exe/turbopump/turbopump";
}

TurboRunner::TurboRunner(short port, string programFlags)
	: _port(port)
	, _dataChannel("/tmp/turbo" + StringUtil::str(port))
	, _programFlags(programFlags)
	, _workingDir(StringUtil::str(port))
{
	boost::filesystem::create_directory(_workingDir);
	createMemberFile(_port);
}

TurboRunner::~TurboRunner()
{
	stop();
	boost::filesystem::remove_all(_workingDir);
}

short TurboRunner::port() const
{
	return _port;
}

string TurboRunner::dataChannel() const
{
	return _dataChannel;
}

void TurboRunner::start()
{
	string command = ("cd " + _workingDir + " && " + exePath + " -p " + StringUtil::str(_port) + " -d " + _dataChannel + " " + _programFlags + " &");
	int res = system(command.c_str());
}

void TurboRunner::stop()
{
	string command = "kill -9 `ps faux | grep 'turbopump ' | grep -v grep | grep '" + _dataChannel + "'" + " | awk '{print $2}'`";
	int res = system(command.c_str());
}

std::string TurboRunner::query(std::string action) const
{
	return CommandLine::run("echo '" + action + "||' | nc -U " + dataChannel());
}

std::string TurboRunner::local_list() const
{
	string response = query("local_list");
	vector<string> files = StringUtil::split(response, '\n');
	std::sort(files.begin(), files.end());
	return StringUtil::join(files, '\n');
}

bool TurboRunner::waitForRunning(unsigned seconds) const
{
	Timer t;
	while (t.millis() < seconds*1000)
	{
		string response = query("state");
		if (response == "running")
			return true;
		std::cout << response << std::endl;
		CommandLine::run("sleep 0.25");
	}
	return false;
}

void TurboRunner::createMemberFile(short firstPort, int firstUid, int members)
{
	Membership membership(_workingDir + "/turbo_members.txt", "localhost:" + _workingDir);
	if (firstUid <= 0)
		firstUid = firstPort;
	for (int i = 0; i < members; ++i)
	{
		string uid = StringUtil::str(firstUid+i);
		string port = StringUtil::str(firstPort+i);
		membership.add(uid);
		membership.addIp("127.0.0.1:" + port, uid);
	}
	membership.save();
}

