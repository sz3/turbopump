/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "integration/TurboRunner.h"
#include "serialize/StringUtil.h"

using std::string;

TEST_CASE( "PartitionStoreTest/testManyWorkers", "[integration-udp]" )
{
	TurboRunner::createMemberFile(5);
	TurboRunner runner1(9001, "/tmp/turbo9001");
	TurboRunner runner2(9002, "/tmp/turbo9002");
	TurboRunner runner3(9003, "/tmp/turbo9003");
	TurboRunner runner4(9004, "/tmp/turbo9004");
	TurboRunner runner5(9005, "/tmp/turbo9005");
	runner1.start();
	runner2.start();
	runner3.start();
	runner4.start();
	runner5.start();

	// wait for everything to run... this sucks... add a status command!
	CommandLine::run("sleep 1");

	string response = CommandLine::run("echo 'membership||' | nc -U /tmp/turbo9001");
	assertEquals( "1 127.0.0.1:9001\n"
				  "2 127.0.0.1:9002\n"
				  "3 127.0.0.1:9003\n"
				  "4 127.0.0.1:9004\n"
				  "5 127.0.0.1:9005", response );

	for (unsigned i = 0; i < 5; ++i)
	{
		string num = StringUtil::str(i);
		response = CommandLine::run("echo 'write|name=file" + num + "|hello" + num + "' | nc -U /tmp/turbo9001");
		assertEquals( "", response );
	}

	// wait for files to propagate
	// TODO: don't
	CommandLine::run("sleep 0.5");

	response = CommandLine::run("echo 'local_list||' | nc -U " + runner2.dataChannel());
	std::cout << "runner2 says " << response << std::endl;
	response = CommandLine::run("echo 'local_list||' | nc -U " + runner3.dataChannel());
	std::cout << "runner3 says " << response << std::endl;
	response = CommandLine::run("echo 'local_list||' | nc -U " + runner4.dataChannel());
	std::cout << "runner4 says " << response << std::endl;
	response = CommandLine::run("echo 'local_list||' | nc -U " + runner5.dataChannel());
	std::cout << "runner5 says " << response << std::endl;
}
