#include "unittest.h"

#include "command_line/CommandLine.h"
#include "membership/Membership.h"

#include "serialize/StringUtil.h"
#include "time/Timer.h"
#include <iostream>
#include <string>
#include <vector>
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

	int openStreamSocket(const string& where)
	{
		struct sockaddr_un address;
		memset(&address, 0, sizeof(struct sockaddr_un));
		address.sun_family = AF_UNIX;
		snprintf(address.sun_path, where.size()+1, where.c_str());

		int socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
		assertTrue( socket_fd >= 0 );
		assertTrue( connect(socket_fd, (struct sockaddr*)&address, sizeof(struct sockaddr_un)) == 0 );

		return socket_fd;
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

	Timer elapsed;
	for (int i = 0; i < 100; ++i)
	{
		int socket_fd = openStreamSocket(runner1.dataChannel());
		std::cout << "write " << i <<  " connection open at " << elapsed.micros() << "us" << std::endl;

		string name = StringUtil::str(i);
		string packet = string("write|name=") + name + "|" + name;
		//packet += StringUtil::str(packet.size()) + packet + '\n';
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		std::cout << "write " << i <<  " finished at " << elapsed.micros() << "us" << std::endl;

		close(socket_fd);
		std::cout << "write " << i <<  " connection close at " << elapsed.micros() << "us" << std::endl;
	}
	std::cout << "did 100 writes in " << elapsed.millis() << "ms" << std::endl;

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

	// check contents of each file?
}

TEST_CASE( "ReadWriteLoadTest/testBigWrite", "[integration]" )
{
	std::vector<string> timingData;

	createMemberFile(2);

	TurboRunner runner1(9001, "/tmp/turbo9001");
	TurboRunner runner2(9002, "/tmp/turbo9002");
	runner1.start();
	runner2.start();

	// wait for everything to run... this sucks... add a status command!
	CommandLine::run("sleep 1");

	const unsigned bufsize = 65536;
	char buffer[bufsize];

	Timer elapsed;
	{
		int socket_fd = openStreamSocket(runner1.dataChannel());
		timingData.push_back("opened write socket at " + StringUtil::str(elapsed.micros()) + "us");

		string packet = "write|name=bigfile|";
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		for (unsigned c = 0; c < bufsize; ++c)
			buffer[c] = 48+(c>>10);
		bytesWrit = write(socket_fd, buffer, bufsize);
		assertEquals(bufsize, bytesWrit);
		close(socket_fd);
	}
	timingData.push_back("finished write at " + StringUtil::str(elapsed.micros()) + "us");
	//std::cout << "finished write at " << elapsed.micros() << "us" << std::endl;

	string expectedContents = string(buffer, bufsize);
	memset(buffer, 0, bufsize);

	{
		int socket_fd = openStreamSocket(runner1.dataChannel());
		timingData.push_back("opened read socket at " + StringUtil::str(elapsed.micros()) + "us");
		//std::cout << "opened read socket at " << elapsed.micros() << "us" << std::endl;

		string packet = "read|name=bigfile|";
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		size_t totalBytes = 0;
		int bytesRead = 0;
		while ((bytesRead = read(socket_fd, buffer+totalBytes, bufsize-totalBytes)) > 0)
			totalBytes += bytesRead;

		assertEquals(bufsize, totalBytes);
		close(socket_fd);
	}

	timingData.push_back("finished read 1 at " + StringUtil::str(elapsed.micros()) + "us");
	//std::cout << "finished read 1 at " << elapsed.micros() << "us" << std::endl;
	string actualContents = string(buffer, bufsize);
	assertEquals( expectedContents, actualContents );


	actualContents = CommandLine::run("echo 'read|name=bigfile|' | nc -U " + runner1.dataChannel());
	assertEquals( expectedContents, actualContents );

	timingData.push_back("finished read 2 at " + StringUtil::str(elapsed.micros()) + "us");
	//std::cout << "finished read 2 at " << elapsed.micros() << "us" << std::endl;


	Timer t;
	while (t.millis() < 10000)
	{
		actualContents = CommandLine::run("echo 'read|name=bigfile|' | nc -U " + runner2.dataChannel());
		if (!actualContents.empty())
			break;
	}
	timingData.push_back("finished read 3 at " + StringUtil::str(elapsed.millis()) + "ms");
	//std::cout << "read 3 finished at " << elapsed.millis() << "ms" << std::endl;
	assertEquals( expectedContents, actualContents );

	for (std::vector<string>::const_iterator it = timingData.begin(); it != timingData.end(); ++it)
		std::cout << *it << std::endl;
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

	const unsigned bufsize = 65536;
	char buffer[bufsize];
	for (int i = 32; i < 122; ++i)
	{
		int socket_fd = openStreamSocket(runner1.dataChannel());

		string name = StringUtil::str(i);
		string packet = string("write|name=") + name + "|";
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		for (unsigned c = 0; c < bufsize; ++c)
			buffer[c] = i;
		bytesWrit = write(socket_fd, buffer, bufsize);
		assertEquals(bufsize, bytesWrit);
		close(socket_fd);
	}

	std::vector<string> results;
	Timer t;
	while (t.millis() < 30000)
	{
		string response = CommandLine::run("echo 'local_list||' | nc -U " + runner2.dataChannel());
		results = StringUtil::split(response, '\n');
		if (results.size() == 90)
			break;
		CommandLine::run("sleep 1");
	}
	assertEquals( 90, results.size() );

	std::vector<string> badResults;
	for (std::vector<string>::const_iterator it = results.begin(); it != results.end(); ++it)
	{
		if (it->find("65536") == string::npos)
			badResults.push_back(*it);
	}
	assertEquals( "", StringUtil::stlJoin(badResults, '\n') );

	// if we got this far, we would then check the contents of each file
}