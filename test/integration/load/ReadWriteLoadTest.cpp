#include "unittest.h"

#include "command_line/CommandLine.h"
#include "membership/Membership.h"
#include "serialize/StringUtil.h"
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using std::string;

namespace
{
	string exePath = "../../../src/exe/turbopump/turbopump";

	class TurboRunner
	{
	public:
		TurboRunner(short port, string dataChannel)
			: _port(port)
			, _dataChannel(dataChannel)
		{}

		short port() const
		{
			return _port;
		}

		string dataChannel() const
		{
			return _dataChannel;
		}

		void start()
		{
			string command = (exePath + " -p " + StringUtil::str(_port) + " -d " + _dataChannel + "&");
			system(command.c_str());
		}

	protected:
		short _port;
		string _dataChannel;
	};

	void createMemberFile(int members)
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
}


TEST_CASE( "ReadWriteLoadTest/testDefault", "[integration]" )
{
	createMemberFile(2);

	system("kill -9 `ps faux | grep turbopump | grep -v grep | awk '{print $2}'`");
	TurboRunner runner1(9001, "/tmp/turbo9001");
	TurboRunner runner2(9002, "/tmp/turbo9002");
	runner1.start();
	runner2.start();

	// wait for everything to run... this sucks...
	CommandLine::run("sleep 1");

	struct sockaddr_un address;
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, runner1.dataChannel().size()+1, runner1.dataChannel().c_str());

	for (int i = 0; i < 100; ++i)
	{
		int socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
		assertTrue( socket_fd >= 0 );
		assertTrue( connect(socket_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_un)) == 0 );

		string name = StringUtil::str(i);
		string packet = string("write|name=") + name + "|" + name;
		write(socket_fd, packet.data(), packet.size());
		close(socket_fd);
	}

	CommandLine::run("sleep 30");

}
