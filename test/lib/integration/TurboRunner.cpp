#include "TurboRunner.h"

#include "membership/Membership.h"
#include "serialize/StringUtil.h"
using std::string;

namespace {
	// TODO: find based on cwd
	string exePath = "../../../src/exe/turbopump/turbopump";
}

TurboRunner::TurboRunner(short port, string dataChannel)
	: _port(port)
	, _dataChannel(dataChannel)
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
	string command = (exePath + " -p " + StringUtil::str(_port) + " -d " + _dataChannel + "&");
	int res = system(command.c_str());
}

void TurboRunner::stop()
{
	int res = system("kill -9 `ps faux | grep turbopump | grep -v grep | awk '{print $2}'`");
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

