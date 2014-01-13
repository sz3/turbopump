/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "common/KeyMetadata.h"
#include "main/TurboPumpApp.h"

#include "command_line/CommandLine.h"
#include "serialize/StringUtil.h"
#include "time/Timer.h"
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
using std::shared_ptr;
using std::string;

class TurboRunner
{
public:
	TurboRunner(TurboPumpApp& worker)
		: _worker(worker)
		, _thread(std::bind(&TurboPumpApp::run, &worker))
	{}

	~TurboRunner()
	{
		_worker.shutdown();
		_thread.join();
	}

protected:
	TurboPumpApp& _worker;
	std::thread _thread;
};

namespace {
	void createMemberFile()
	{
		Membership membership("turbo_members.txt", "localhost:1337");
		membership.add("one");
		membership.addIp("127.0.0.1:9001", "one");
		membership.add("two");
		membership.addIp("127.0.0.1:9002", "two");
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

TEST_CASE( "StartupTest/testMerkleHealing", "[integration]" )
{
	createMemberFile();

	TurboApi api;
	api.options.merkle = true;
	api.options.write_chaining = false;
	api.options.partition_keys = false;
	TurboPumpApp workerOne(api, "/tmp/workerOne", 9001);
	TurboRunner runnerOne(workerOne);

	TurboPumpApp workerTwo(api, "/tmp/workerTwo", 9002);
	TurboRunner runnerTwo(workerTwo);

	// TODO: make this sleep go away
	CommandLine::run("sleep 0.5");

	// test stuff!

	string response = CommandLine::run("echo 'membership||' | nc -U /tmp/workerOne");
	assertEquals( "one 127.0.0.1:9001\n"
				  "two 127.0.0.1:9002", response );

	for (unsigned i = 0; i < 5; ++i)
	{
		string num = StringUtil::str(i);
		response = CommandLine::run("echo 'write|name=one" + num + "|one hello" + num + "' | nc -U /tmp/workerOne");
		assertEquals( "", response );
	}
	for (unsigned i = 0; i < 5; ++i)
	{
		string num = StringUtil::str(i);
		response = CommandLine::run("echo 'write|name=two" + num + "|two hello" + num + "' | nc -U /tmp/workerTwo");
		assertEquals( "", response );
	}

	CommandLine::run("sleep 10");

	string expected = "(one0)=>11\n"
					  "(one1)=>11\n"
					  "(one2)=>11\n"
					  "(one3)=>11\n"
					  "(one4)=>11\n"
					  "(two0)=>11\n"
					  "(two1)=>11\n"
					  "(two2)=>11\n"
					  "(two3)=>11\n"
					  "(two4)=>11";

	response = CommandLine::run("echo 'local_list||' | nc -U /tmp/workerTwo");
	assertEquals( expected, response );

	response = CommandLine::run("echo 'local_list||' | nc -U /tmp/workerOne");
	assertEquals( expected, response );
}

namespace
{
	class Checkpoint
	{
	public:
		Checkpoint() {}

		void reset()
		{
			_timer.reset();
		}

		void add()
		{
			_results.push_back(_timer.micros());
		}

		const Timer& timer() const
		{
			return _timer;
		}

		const std::vector<long long>& results() const
		{
			return _results;
		}

	protected:
		Timer _timer;
		std::vector<long long> _results;
	};
}

TEST_CASE( "StartupTest/testWriteChaining", "[integration]" )
{
	const unsigned numFiles = 5;
	createMemberFile();

	TurboApi api;
	api.options.merkle = false;
	api.options.write_chaining = true;
	api.options.partition_keys = false;

	// create all checkpoints while we're on an orderly single thread.
	// the callbacks will be chaotic, but as long as all our map allocations are done, we should be fine
	std::map<string,Checkpoint> checkpoints;
	for (unsigned i = 0; i < numFiles; ++i)
		checkpoints[StringUtil::str(i)];

	api.when_local_write_finishes = api.when_mirror_write_finishes = [&checkpoints] (KeyMetadata md, IDataStoreReader::ptr)
	{
		checkpoints[md.filename].add();
	};

	TurboPumpApp workerOne(api, "/tmp/workerOne", 9001);
	TurboRunner runnerOne(workerOne);

	TurboPumpApp workerTwo(api, "/tmp/workerTwo", 9002);
	TurboRunner runnerTwo(workerTwo);

	// need some events to signal when we're running...
	CommandLine::run("sleep 1");

	// test stuff!
	CommandLine::run("echo 'write|name=primer|priming the wan pump' | nc -U /tmp/workerOne");
	CommandLine::run("sleep 0.2");

	for (unsigned i = 0; i < numFiles; ++i)
	{
		string num = StringUtil::str(i);
		checkpoints[num].reset();

		string packet = "write|name=" + num + "|hello" + num + "\n";

		int socket_fd = openStreamSocket("/tmp/workerOne");
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());
		close(socket_fd);

		std::cout << "local write " << i <<  " finished at " << checkpoints[num].timer().micros() << "us" << std::endl;
	}

	string expected = "(0)=>7\n"
			"(1)=>7\n"
			"(2)=>7\n"
			"(3)=>7\n"
			"(4)=>7\n"
			"(primer)=>21";

	string response;
	Timer t;
	while (t.millis() < 5000)
	{
		response = CommandLine::run("echo 'local_list||' | nc -U /tmp/workerTwo");
		if (response == expected)
			break;
		CommandLine::run("sleep 1");
	}
	assertEquals( expected, response );

	for (std::map<string,Checkpoint>::const_iterator it = checkpoints.begin(); it != checkpoints.end(); ++it)
		std::cout << "  " << it->first << " : " << StringUtil::stlJoin(it->second.results()) << std::endl;
}

