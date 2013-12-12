#include "unittest.h"

#include "main/TurboPumpApp.h"

#include "command_line/CommandLine.h"
#include "serialize/StringUtil.h"
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

TEST_CASE( "StartupTest/testDefault", "[integration]" )
{
	createMemberFile();

	TurboApi api;
	TurboPumpApp workerOne(api, "/tmp/workerOne", 9001);
	TurboRunner runnerOne(workerOne);

	TurboPumpApp workerTwo(api, "/tmp/workerTwo", 9002);
	TurboRunner runnerTwo(workerTwo);

	// need some events to signal when we're running...
	CommandLine::run("sleep 1");

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

