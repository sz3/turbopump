/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "api/WriteInstructions.h"
#include "integration/TurboRunner.h"
#include "main/TurboPumpApp.h"

#include "command_line/CommandLine.h"
#include "serialize/StringUtil.h"
#include "serialize/str_join.h"
#include "time/stopwatch.h"
#include "time/WaitFor.h"
#include <array>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <unistd.h>
using std::shared_ptr;
using std::string;

class IntegratedTurboRunner : public TurboRunner
{
public:
	IntegratedTurboRunner(const Turbopump::Options& opts, short port)
		: TurboRunner(port)
		, _app(opts, dataChannel(), port)
		, _thread(std::bind(&TurboPumpApp::run, &_app))
	{
		::signal(SIGPIPE, SIG_IGN);
	}

	~IntegratedTurboRunner()
	{
		_app.shutdown();
		_thread.join();
	}

protected:
	TurboPumpApp _app;
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

	Turbopump::Options opts;
	opts.active_sync = true;
	opts.write_chaining = false;
	opts.partition_keys = false;

	IntegratedTurboRunner workerOne(opts, 9001);
	IntegratedTurboRunner workerTwo(opts, 9002);

	workerOne.waitForRunning();
	workerTwo.waitForRunning();

	string response = workerOne.query("membership");
	assertEquals( "one 127.0.0.1:9001\n"
				  "two 127.0.0.1:9002", response );

	for (unsigned i = 0; i < 5; ++i)
	{
		string num = StringUtil::str(i);
		response = workerOne.write("one" + num, "one hello " + num);
		assertEquals( "200", response );
	}
	for (unsigned i = 0; i < 5; ++i)
	{
		string num = StringUtil::str(i);
		response = workerTwo.write("two" + num, "two hello " + num);
		assertEquals( "200", response );
	}

	// we're running inside a single executable, and two initializes second. So, all writes will increment with member id "two".
	string expected = "(one0)=>11|1,two:1\n"
	                  "(one1)=>11|1,two:1\n"
	                  "(one2)=>11|1,two:1\n"
	                  "(one3)=>11|1,two:1\n"
	                  "(one4)=>11|1,two:1\n"
	                  "(two0)=>11|1,two:1\n"
	                  "(two1)=>11|1,two:1\n"
	                  "(two2)=>11|1,two:1\n"
	                  "(two3)=>11|1,two:1\n"
	                  "(two4)=>11|1,two:1";

	waitFor(5, response + " != " + expected, [&]()
	{
		response = workerTwo.local_list();
		return expected == response;
	});

	waitFor(5, response + " != " + expected, [&]()
	{
		response = workerOne.local_list();
		return expected == response;
	});
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

		const stopwatch& timer() const
		{
			return _timer;
		}

		const std::vector<long long>& results() const
		{
			return _results;
		}

	protected:
		stopwatch _timer;
		std::vector<long long> _results;
	};
}

TEST_CASE( "StartupTest/testWriteChaining", "[integration]" )
{
	const unsigned numFiles = 5;
	createMemberFile();

	Turbopump::Options opts;
	opts.active_sync = false;
	opts.write_chaining = true;
	opts.partition_keys = false;

	// create all checkpoints while we're on an orderly single thread.
	// the callbacks will be chaotic, but as long as all our map allocations are done, we should be fine
	std::map<string,Checkpoint> checkpoints;
	for (unsigned i = 0; i < numFiles; ++i)
		checkpoints[StringUtil::str(i)];

	opts.when_local_write_finishes = opts.when_mirror_write_finishes = [&checkpoints] (WriteInstructions& params, IDataStoreReader::ptr)
	{
		checkpoints[params.name].add();
	};

	IntegratedTurboRunner workerOne(opts, 9001);
	IntegratedTurboRunner workerTwo(opts, 9002);

	workerOne.waitForRunning();
	workerTwo.waitForRunning();

	// test stuff!
	workerOne.write("primer", "priming the wan pump");
	CommandLine::run("sleep 0.2");

	char readBuff[100];
	for (unsigned i = 0; i < numFiles; ++i)
	{
		string num = StringUtil::str(i);
		checkpoints[num].reset();

		string data = "hello " + num;
		string packet = TurboRunner::headerForWrite(num, data.size()) + data;

		int socket_fd = openStreamSocket(workerOne.dataChannel());
		size_t bytesWrit = write(socket_fd, packet.data(), packet.size());

		size_t bytesRead = read(socket_fd, readBuff, 100);
		close(socket_fd);

		assertStringContains( "200 Success", string(readBuff, bytesRead) );

		std::cout << "local write " << i <<  " finished at " << checkpoints[num].timer().micros() << "us" << std::endl;
	}

	// we're running inside a single executable, and two initializes second. So, all writes will increment with member id "two".
	string expected = "(0)=>7|1,two:1\n"
	                  "(1)=>7|1,two:1\n"
	                  "(2)=>7|1,two:1\n"
	                  "(3)=>7|1,two:1\n"
	                  "(4)=>7|1,two:1\n"
					  "(primer)=>20|1,two:1";

	string response;
	waitFor(5, response + " != " + expected, [&]()
	{
		response = workerTwo.local_list();
		return expected == response;
	});

	for (std::map<string,Checkpoint>::const_iterator it = checkpoints.begin(); it != checkpoints.end(); ++it)
		std::cout << "  " << it->first << " : " << turbo::str::join(it->second.results()) << std::endl;
}

TEST_CASE( "StartupTest/testWriteBigFile", "[integration]" )
{
	::signal(SIGPIPE, SIG_IGN);
	createMemberFile();

	Turbopump::Options opts;
	opts.active_sync = false;
	opts.write_chaining = true;
	opts.partition_keys = false;

	IntegratedTurboRunner workerOne(opts, 9001);
	IntegratedTurboRunner workerTwo(opts, 9002);

	workerOne.waitForRunning();
	workerTwo.waitForRunning();

	char readBuff[100];
	{
		string data;
		data.resize(1024, 'a');
		string header = TurboRunner::headerForWrite("0", data.size()*65);

		int socket_fd = openStreamSocket(workerOne.dataChannel());
		size_t bytesWrit = write(socket_fd, header.data(), header.size());
		for (unsigned i = 0; i < 65; ++i)
			bytesWrit = write(socket_fd, data.data(), data.size());

		size_t bytesRead = read(socket_fd, readBuff, 100);
		close(socket_fd);

		assertStringContains( "200 Success", string(readBuff, bytesRead) );
	}

	string expected = "(0)=>66560|1,two:1";
	string response = workerOne.local_list();
	assertEquals(expected, response);

	expected = "(0)=>66560|1,two:1";
	waitFor(5, response + " != " + expected, [&]()
	{
		response = workerTwo.local_list();
		return expected == response;
	});
}

