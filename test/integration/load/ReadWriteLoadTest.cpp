#include "unittest.h"

#include "command_line/CommandLine.h"
#include "membership/Membership.h"

#include "serialize/StringUtil.h"
#include "time/Timer.h"
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

		~TurboRunner()
		{
			stop();
		}

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
			int res = system(command.c_str());
		}

		void stop()
		{
			int res = system("kill -9 `ps faux | grep turbopump | grep -v grep | awk '{print $2}'`");
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

TEST_CASE( "ReadWriteLoadTest/testSmallWrites", "[integration]" )
{
	createMemberFile(2);

	TurboRunner runner1(9001, "/tmp/turbo9001");
	TurboRunner runner2(9002, "/tmp/turbo9002");
	runner1.start();
	runner2.start();

	// wait for everything to run... this sucks... add a status command!
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
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());
		close(socket_fd);
	}

	string response;
	Timer t;
	while (t.millis() < 30000)
	{
		response = CommandLine::run("echo 'local_list||' | nc -U " + runner2.dataChannel() + " | wc | awk '{print $2}'");
		if (response == "100\n")
			break;
		CommandLine::run("sleep 1");
	}
	assertEquals( "100\n", response );
}

TEST_CASE( "ReadWriteLoadTest/testBigWrite", "[integration]" )
{
	createMemberFile(2);

	TurboRunner runner1(9001, "/tmp/turbo9001");
	TurboRunner runner2(9002, "/tmp/turbo9002");
	runner1.start();
	runner2.start();

	// wait for everything to run... this sucks... add a status command!
	CommandLine::run("sleep 1");

	struct sockaddr_un address;
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, runner1.dataChannel().size()+1, runner1.dataChannel().c_str());

	const unsigned bufsize = 65536;
	char buffer[bufsize];

	{
		int socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
		assertTrue( socket_fd >= 0 );
		assertTrue( connect(socket_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_un)) == 0 );

		string name = StringUtil::str("bigfile");
		string packet = string("write|name=") + name + "|";
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		for (unsigned c = 0; c < bufsize; ++c)
			buffer[c] = 'w';
		bytesWrit = write(socket_fd, buffer, bufsize);
		assertEquals(bufsize, bytesWrit);
		close(socket_fd);
	}

	CommandLine::run("sleep 30");
}

TEST_CASE( "ReadWriteLoadTest/testManyBigWrites", "[integration]" )
{
	createMemberFile(2);

	TurboRunner runner1(9001, "/tmp/turbo9001");
	TurboRunner runner2(9002, "/tmp/turbo9002");
	runner1.start();
	runner2.start();

	// wait for everything to run... this sucks... add a status command!
	CommandLine::run("sleep 1");

	struct sockaddr_un address;
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, runner1.dataChannel().size()+1, runner1.dataChannel().c_str());

	const unsigned bufsize = 65536;
	char buffer[bufsize];
	for (int i = 32; i < 122; ++i)
	{
		int socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
		assertTrue( socket_fd >= 0 );
		assertTrue( connect(socket_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_un)) == 0 );

		string name = StringUtil::str(i);
		string packet = string("write|name=") + name + "|";
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		for (unsigned c = 0; c < bufsize; ++c)
			buffer[c] = i;
		bytesWrit = write(socket_fd, buffer, bufsize);
		assertEquals(bufsize, bytesWrit);
		close(socket_fd);
	}

	// do stuff
}
