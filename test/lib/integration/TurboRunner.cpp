/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "TurboRunner.h"
#include "membership/Membership.h"

#include "http/HttpResponse.h"
#include "serialize/str.h"
#include "system/popen.h"
#include "time/stopwatch.h"

#include "boost/filesystem.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
using std::string;
using std::map;
using std::vector;
using turbo::str::str;

namespace {
	string exePath = string(TURBOPUMP_PROJECT_ROOT) + "/build/src/exe/turbopumpd/turbopumpd";

	std::ostream& operator<<(std::ostream& outstream, const std::pair<std::string,std::string>& val)
	{
		outstream << val.first << " => " << val.second;
		return outstream;
	}
}
#include "serialize/str_join.h"

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
	string command = ("cd " + _workingDir + " && " + exePath + " -p " + str(_port) + " -l " + _dataChannel + " " + _programFlags + " &");
	//std::cout << " starting " << command << std::endl;
	int res = system(command.c_str());
}

bool TurboRunner::stop(unsigned retries)
{
	string command = "kill `ps faux | grep 'turbopumpd ' | grep -v grep | grep '" + _dataChannel + "'" + " | awk '{print $2}'`";
	for (unsigned i = 0; i < retries; ++i)
	{
		int res = system(command.c_str());
		if (res == 0)
			return true;
		//std::cout << "stop " << i << " (" << command << ") returned " << res << std::endl;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	return false;
}

std::string TurboRunner::query(std::string action, std::string params) const
{
	string response = turbo::popen("echo 'GET /" + action + (params.empty()? "" : "?" + params) + " HTTP/1.1\r\n\r\n' | nc -U " + dataChannel()).read();
	return HttpResponse().parse(response).body();
}

std::string TurboRunner::post(std::string action, std::string params, std::string body) const
{
	string response = turbo::popen("echo 'POST /" + action + (params.empty()? "" : "?" + params) + " HTTP/1.1\r\n"
		   "content-length:" + str(body.size()) + "\r\n" + body + "' | nc -U " + dataChannel()).read();
	return HttpResponse().parse(response).status().str();
}

std::string TurboRunner::local_list(std::string params) const
{
	map<string,string> list = list_keys(params);
	return turbo::str::join(list, '\n');
}

std::map<std::string,std::string> TurboRunner::list_keys(std::string params) const
{
	string body = query("list-keys", params);
	vector<string> files = turbo::str::split(body, '\n');

	map<string,string> list;
	for (auto line : files)
	{
		vector<string> tokens = turbo::str::split(line, '"', true);
		if (tokens.size() < 3)
			continue;
		string& name(tokens[0]);
		string& info(tokens[2]);
		list[name] = info;
	}
	return list;
}

bool TurboRunner::waitForRunning(unsigned seconds) const
{
	return waitForState("running", seconds);
}

bool TurboRunner::waitForState(std::string state, unsigned seconds) const
{
	turbo::stopwatch t;
	while (t.millis() < seconds*1000)
	{
		string response = query("status");
		if (response == state)
			return true;
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}
	std::cout << "waitForState '" << state << "' failed after " << seconds << "s" << std::endl;
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
	string res = turbo::popen("echo '" + req + "' | nc -U " + dataChannel()).read();
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

