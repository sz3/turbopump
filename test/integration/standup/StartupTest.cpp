#include "unittest.h"

#include "main/TurboPumpApp.h"

#include "command_line/CommandLine.h"
#include <functional>
#include <memory>
#include <string>
#include <thread>
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
}

TEST_CASE( "StartupTest/testDefault", "default" )
{
	createMemberFile();

	TurboApi api;
	TurboPumpApp workerOne(api, "/tmp/workerOne", 9001);
	TurboRunner runnerOne(workerOne);

	TurboPumpApp workerTwo(api, "/tmp/workerTwo", 9002);
	TurboRunner runnerTwo(workerTwo);

	// test stuff!

	string response = CommandLine::run("echo 'membership||' | nc -U /tmp/workerOne");
	assertEquals( "one 127.0.0.1:9001\n"
				  "two 127.0.0.1:9002", response );

	response = CommandLine::run("echo 'write|name=foo|hello' | nc -U /tmp/workerTwo");
	assertEquals( "", response );

	CommandLine::run("sleep 10");

	response = CommandLine::run("echo 'local_list||' | nc -U /tmp/workerOne");
	assertEquals( "hi", response );
}

