/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "turbopumpd/app.h"

#include "api/WriteInstructions.h"
#include "integration/TurboRunner.h"

#include "serialize/format.h"
#include "serialize/str.h"
#include "serialize/str_join.h"
#include "time/stopwatch.h"
#include "time/wait_for.h"
#include <array>
#include <chrono>
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
using turbo::stopwatch;
using turbo::str::str;
using turbo::wait_for_equal;
using turbo::wait_for_match;

class IntegratedTurboRunner : public TurboRunner
{
public:
	IntegratedTurboRunner(const Turbopump::Options& opts)
		: TurboRunner(opts.internal_port)
		, _app(opts, socket_address(dataChannel()))
		, _thread(std::bind(&TurboApp::run, &_app))
	{
		::signal(SIGPIPE, SIG_IGN);
	}

	~IntegratedTurboRunner()
	{
		_app.shutdown();
		_thread.join();
	}

protected:
	TurboApp _app;
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

	opts.internal_port = 9001;
	IntegratedTurboRunner workerOne(opts);
	opts.internal_port = 9002;
	IntegratedTurboRunner workerTwo(opts);

	workerOne.waitForRunning();
	workerTwo.waitForRunning();

	string response = workerOne.query("membership");
	assertEquals( "one 127.0.0.1:9001\n"
				  "two 127.0.0.1:9002", response );

	for (unsigned i = 0; i < 5; ++i)
	{
		string num = str(i);
		response = workerOne.write("one" + num, "one hello " + num);
		assertEquals( "200", response );
	}
	for (unsigned i = 0; i < 5; ++i)
	{
		string num = str(i);
		response = workerTwo.write("two" + num, "two hello " + num);
		assertEquals( "200", response );
	}

	// we're running inside a single executable, and two initializes second. So, all writes will increment with member id "two".
	string expected = fmt::format(
		"one0 => 11:1,two.{0}\n"
		"one1 => 11:1,two.{0}\n"
		"one2 => 11:1,two.{0}\n"
		"one3 => 11:1,two.{0}\n"
		"one4 => 11:1,two.{0}\n"
		"two0 => 11:1,two.{0}\n"
		"two1 => 11:1,two.{0}\n"
		"two2 => 11:1,two.{0}\n"
		"two3 => 11:1,two.{0}\n"
		"two4 => 11:1,two.{0}"
		, "([^\\. ]+)"
	);

	wait_for_match(5, expected, [&]()
	{
		return workerTwo.local_list();
	});

	wait_for_match(5, expected, [&]()
	{
		return workerOne.local_list();
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
		checkpoints[str(i)];

	opts.when_local_write_finishes = opts.when_mirror_write_finishes = [&checkpoints] (WriteInstructions& params, readstream&)
	{
		checkpoints[params.name].add();
	};

	opts.internal_port = 9001;
	IntegratedTurboRunner workerOne(opts);
	opts.internal_port = 9002;
	IntegratedTurboRunner workerTwo(opts);

	workerOne.waitForRunning();
	workerTwo.waitForRunning();

	// test stuff!
	workerOne.write("primer", "priming the wan pump");
	std::this_thread::sleep_for(std::chrono::milliseconds(200));

	char readBuff[100];
	for (unsigned i = 0; i < numFiles; ++i)
	{
		string num = str(i);
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
	string expected = fmt::format(
		"0 => 7:1,two.{0}\n"
		"1 => 7:1,two.{0}\n"
		"2 => 7:1,two.{0}\n"
		"3 => 7:1,two.{0}\n"
		"4 => 7:1,two.{0}\n"
		"primer => 20:1,two.{0}"
		, "([^\\. ]+)"
	);

	wait_for_match(5, expected, [&]()
	{
		return workerTwo.local_list();
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

	opts.internal_port = 9001;
	IntegratedTurboRunner workerOne(opts);
	opts.internal_port = 9002;
	IntegratedTurboRunner workerTwo(opts);

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

	string expected = "0 => 66560:1,two.([^\\. ]+)";
	string response = workerOne.local_list();
	assertMatch(expected, response);

	expected = response;
	wait_for_equal(5, expected, [&]()
	{
		return workerTwo.local_list();
	});
}

