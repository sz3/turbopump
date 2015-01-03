/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboRunner.h"
#include "membership/Membership.h"

#include "command_line/CommandLine.h"
#include "http/HttpResponse.h"
#include "serialize/str.h"
#include "serialize/str_join.h"
#include "time/stopwatch.h"

#include "boost/filesystem.hpp"
#include <iostream>
#include <vector>
using std::string;
using std::vector;
using turbo::str::str;

namespace {
	string exePath = string(TURBOPUMP_PROJECT_ROOT) + "/build/src/exe/turbopump/turbopump";
}

TurboRunner::TurboRunner(short port, string programFlags)
	: _port(port)
	, _dataChannel("/tmp/turbo" + str(port))
	, _programFlags(programFlags)
	, _workingDir(str(port))
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
	string command = ("cd " + _workingDir + " && " + exePath + " -p " + str(_port) + " -d " + _dataChannel + " " + _programFlags + " &");
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
		   "content-length:" + str(body.size()) + "\r\n" + body + "' | nc -U " + dataChannel());
	return HttpResponse().parse(response).status().str();
}

std::string TurboRunner::local_list(std::string params) const
{
	string body = query("list-keys", params);
	vector<string> files = turbo::str::split(body, '\n');
	if (files.empty())
		return "";
	std::sort(files.begin(), files.end());
	// blank line will be first.
	return turbo::str::join(++files.begin(), files.end(), '\n');
}

bool TurboRunner::waitForRunning(unsigned seconds) const
{
	turbo::stopwatch t;
	while (t.millis() < seconds*1000)
	{
		string response = query("status");
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
			"content-length:" + str(contentLength) + "\r\n\r\n";
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
		string uid = str(firstUid+i);
		string port = str(firstPort+i);
		membership.add(uid);
		membership.addIp("127.0.0.1:" + port, uid);
	}
	membership.save();
}

