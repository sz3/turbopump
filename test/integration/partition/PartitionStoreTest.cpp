/* This code is subject to the terms of the Mozilla Public License, v.2.0. http://mozilla.org/MPL/2.0/. */
#include "unittest.h"

#include "command_line/CommandLine.h"
#include "consistent_hashing/HashRing.h"
#include "integration/TurboRunner.h"
#include "serialize/StringUtil.h"

using std::string;

namespace {
	string hashRingStr(string id)
	{
		return HashRing::hash(id) + ":" + id + " ";
	}
}

TEST_CASE( "PartitionStoreTest/testManyWorkers", "[integration-udp]" )
{
	TurboRunner::createMemberFile(5);
	TurboRunner runner1(9001, "/tmp/turbo9001", "--udp");
	TurboRunner runner2(9002, "/tmp/turbo9002", "--udp");
	TurboRunner runner3(9003, "/tmp/turbo9003", "--udp");
	TurboRunner runner4(9004, "/tmp/turbo9004", "--udp");
	TurboRunner runner5(9005, "/tmp/turbo9005", "--udp");
	runner1.start();
	runner2.start();
	runner3.start();
	runner4.start();
	runner5.start();

	assertTrue( runner1.waitForRunning() );
	assertTrue( runner2.waitForRunning() );
	assertTrue( runner3.waitForRunning() );
	assertTrue( runner4.waitForRunning() );
	assertTrue( runner5.waitForRunning() );

	string response = runner1.query("membership");
	assertEquals( "1 127.0.0.1:9001\n"
				  "2 127.0.0.1:9002\n"
				  "3 127.0.0.1:9003\n"
				  "4 127.0.0.1:9004\n"
				  "5 127.0.0.1:9005", response );

	response = runner1.query("ring");
	string expected = hashRingStr("2") + hashRingStr("1") + hashRingStr("5") + hashRingStr("4") + hashRingStr("3");
	assertEquals(expected, response);

	for (unsigned i = 1; i <= 5; ++i)
	{
		string num = StringUtil::str(i);
		response = CommandLine::run("echo 'write|name=" + num + "|hello" + num + "' | nc -U " + runner1.dataChannel());
		assertEquals( "", response );
	}

	// wait for files to propagate
	// TODO: don't
	CommandLine::run("sleep 0.5");

	response = runner1.query("local_list");
	std::cout << "runner1 says " << response << std::endl;
	response = runner2.query("local_list");
	std::cout << "runner2 says " << response << std::endl;
	response = runner3.query("local_list");
	std::cout << "runner3 says " << response << std::endl;
	response = runner4.query("local_list");
	std::cout << "runner4 says " << response << std::endl;
	response = runner5.query("local_list");
	std::cout << "runner5 says " << response << std::endl;
}
