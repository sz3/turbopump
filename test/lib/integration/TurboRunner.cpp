/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboRunner.h"
#include "command_line/CommandLine.h"
#include "membership/Membership.h"
#include "serialize/StringUtil.h"
#include "time/Timer.h"
#include <iostream>
using std::string;

namespace {
	// TODO: find based on cwd
	string exePath = "../../../src/exe/turbopump/turbopump";
}

TurboRunner::TurboRunner(short port, string dataChannel, std::string programFlags)
	: _port(port)
	, _dataChannel(dataChannel)
	, _programFlags(programFlags)
{}

TurboRunner::~TurboRunner()
{
	stop();
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
	string command = (exePath + " -p " + StringUtil::str(_port) + " -d " + _dataChannel + " " + _programFlags + " &");
	int res = system(command.c_str());
}

void TurboRunner::stop()
{
	int res = system("kill -9 `ps faux | grep 'turbopump ' | grep -v grep | awk '{print $2}'`");
}

std::string TurboRunner::query(std::string action) const
{
	return CommandLine::run("echo '" + action + "||' | nc -U " + dataChannel());
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

void TurboRunner::createMemberFile(int members)
{
	Membership membership("turbo_members.txt", "localhost:1337");
	for (int i = 1; i <= members; ++i)
	{
		string uid = StringUtil::str(i);
		membership.add(uid);
		membership.addIp("127.0.0.1:" + StringUtil::str(9000+i), uid);
	}
	membership.save();
}

