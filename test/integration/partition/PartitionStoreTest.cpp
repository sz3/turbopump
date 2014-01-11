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
	TurboRunner::createMemberFile(6);
	TurboRunner runner1(9001, "/tmp/turbo9001", "--udp");
	TurboRunner runner2(9002, "/tmp/turbo9002", "--udp");
	TurboRunner runner3(9003, "/tmp/turbo9003", "--udp");
	TurboRunner runner4(9004, "/tmp/turbo9004", "--udp");
	TurboRunner runner5(9005, "/tmp/turbo9005", "--udp");
	TurboRunner runner6(9006, "/tmp/turbo9006", "--udp");
	runner1.start();
	runner2.start();
	runner3.start();
	runner4.start();
	runner5.start();
	runner6.start();

	assertTrue( runner1.waitForRunning() );
	assertTrue( runner2.waitForRunning() );
	assertTrue( runner3.waitForRunning() );
	assertTrue( runner4.waitForRunning() );
	assertTrue( runner5.waitForRunning() );
	assertTrue( runner6.waitForRunning() );

	string response = runner1.query("membership");
	assertEquals( "1 127.0.0.1:9001\n"
				  "2 127.0.0.1:9002\n"
				  "3 127.0.0.1:9003\n"
				  "4 127.0.0.1:9004\n"
				  "5 127.0.0.1:9005\n"
				  "6 127.0.0.1:9006", response );

	// the hash ring order is 2, 6, 1, 5, 4, 3.
	// e.g. if the primary location is 2, the secondary is 6, the tertiary is 1...
	string expected = hashRingStr("2") + hashRingStr("6") + hashRingStr("1") + hashRingStr("5") + hashRingStr("4") + hashRingStr("3");
	response = runner1.query("ring");
	assertEquals(expected, response);

	// setting n=3 here, so we get 3 "proper" copies of the file instead of 2. (also, if we decide to change the defaults at any point...)
	for (unsigned i = 1; i <= 6; ++i)
	{
		string num = StringUtil::str(i);
		response = CommandLine::run("echo 'write|name=" + num + " n=3|hello" + num + "' | nc -U " + runner1.dataChannel());
		assertEquals( "", response );
	}

	// wait for files to propagate
	// TODO: don't
	CommandLine::run("sleep 0.5");

	// again, 2,6,1,5,4,3
	response = runner2.query("local_list");
	assertEquals( "(2)=>7\n"
				  "(3)=>7\n"
				  "(4)=>7", response );

	response = runner3.query("local_list");
	assertEquals( "(3)=>7\n"
				  "(4)=>7\n"
				  "(5)=>7", response );

	response = runner4.query("local_list");
	assertEquals( "(1)=>7\n"
				  "(4)=>7\n"
				  "(5)=>7", response );

	response = runner5.query("local_list");
	assertEquals( "(1)=>7\n"
				  "(5)=>7\n"
				  "(6)=>7", response );

	response = runner6.query("local_list");
	assertEquals( "(2)=>7\n"
				  "(3)=>7\n"
				  "(6)=>7", response );

	// runner1 has everything, since cleanup isn't implemented yet...
	response = runner1.query("local_list");
	assertEquals( "(1)=>7\n"
				  "(2)=>7\n"
				  "(3)=>7\n"
				  "(4)=>7\n"
				  "(5)=>7\n"
				  "(6)=>7", response );
}
