/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboRunner.h"
#include "membership/Membership.h"

#include "command_line/CommandLine.h"
#include "http/HttpResponse.h"
#include "serialize/StringUtil.h"
#include "time/stopwatch.h"

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
	string command = "kill `ps faux | grep 'turbopump ' | grep -v grep | grep '" + _dataChannel + "'" + " | awk '{print $2}'`";
	int res = system(command.c_str());
}

std::string TurboRunner::query(std::string action, std::string params) const
{
	string response = CommandLine::run("echo 'GET /" + action + (params.empty()? "" : "?" + params) + " HTTP/1.1\r\n\r\n' | nc -U " + dataChannel());
	return HttpResponse().parse(response).body();
}

std::string TurboRunner::post(std::string action, std::string params, std::string body) const
{
	string response = CommandLine::run("echo 'POST /" + action + (params.empty()? "" : "?" + params) + " HTTP/1.1\r\n"
	       "content-length:" + StringUtil::str(body.size()) + "\r\n" + body + "' | nc -U " + dataChannel());
	return HttpResponse().parse(response).status().str();
}

std::string TurboRunner::local_list(std::string params) const
{
	string body = query("local_list", params);
	vector<string> files = StringUtil::split(body, '\n');
	if (files.empty())
		return "";
	std::sort(files.begin(), files.end());
	return StringUtil::join(files, '\n');
}

bool TurboRunner::waitForRunning(unsigned seconds) const
{
	stopwatch t;
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

std::string TurboRunner::headerForWrite(std::string name, unsigned contentLength, std::string params/*=""*/)
{
	return "POST /write?name=" + name + (params.empty()? "" : "&" + params) + " HTTP/1.1\r\n"
			"content-length:" + StringUtil::str(contentLength) + "\r\n\r\n";
}

std::string TurboRunner::headerForRead(std::string name, std::string params/*=""*/)
{
	return "GET /read?name=" + name + (params.empty()? "" : "&" + params) + " HTTP/1.1\r\n\r\n";
}

std::string TurboRunner::write(std::string name, std::string data, std::string params/*=""*/)
{
	string req = headerForWrite(name, data.size(), params) + data;
	string res = CommandLine::run("echo '" + req + "' | nc -U " + dataChannel());
	return HttpResponse().parse(res).status().str();
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

